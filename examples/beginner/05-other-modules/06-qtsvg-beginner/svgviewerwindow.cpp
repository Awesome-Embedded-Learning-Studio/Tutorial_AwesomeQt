/**
 * QtSvg 矢量图形基础示例
 *
 * 本示例演示 QtSvg 模块的核心用法：
 * 1. QSvgWidget 直接显示 SVG 文件
 * 2. QSvgRenderer 在 QPainter 中渲染 SVG
 * 3. SVG 动态着色（替换 fill 颜色）
 * 4. 访问 SVG 内部元素（elementId）
 * 5. SVG 渲染到 QPixmap 用作图标（高 DPI 适配）
 */

#include <QApplication>
#include <QColorDialog>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QRegularExpression>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QSvgRenderer>
#include <QSvgWidget>
#include <QVBoxLayout>
#include <QXmlStreamReader>
#include <QFileInfo>

#include "svgviewerwindow.h"

/// 将 SVG 渲染到指定尺寸的 QPixmap
static QPixmap renderSvgToPixmap(QSvgRenderer &renderer, int size)
{
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    renderer.render(&painter);
    painter.end();
    return pixmap;
}

/// 替换 SVG 内容中所有 fill="..." 为指定颜色
static QByteArray recolorSvg(const QByteArray &svgData, const QColor &color)
{
    QString content = QString::fromUtf8(svgData);
    QRegularExpression regex(R"(fill="[^"]*")");
    QString replacement = QString("fill=\"%1\"").arg(color.name());
    content.replace(regex, replacement);
    return content.toUtf8();
}

/// 从 SVG 数据中提取所有带 id 属性的元素名称
static QStringList extractElementIds(const QByteArray &svgData)
{
    QStringList ids;
    QXmlStreamReader xml(svgData);
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            QString id = xml.attributes().value("id").toString();
            if (!id.isEmpty()) {
                ids.append(id);
            }
        }
    }
    return ids;
}

// ============================================================================
// 生成一个用于演示的内联 SVG（spritsheet 风格）
// 包含多个带 id 的分组元素
// ============================================================================
static QByteArray createDemoSvg()
{
    return R"(<?xml version="1.0" encoding="UTF-8"?>
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 200 200">
  <g id="icon-circle">
    <circle cx="40" cy="40" r="30" fill="#3498DB"/>
  </g>
  <g id="icon-square">
    <rect x="80" y="10" width="60" height="60" fill="#2ECC71"/>
  </g>
  <g id="icon-triangle">
    <polygon points="40,100 10,160 70,160" fill="#E74C3C"/>
  </g>
  <g id="icon-star">
    <polygon points="130,100 140,130 170,130 145,150 155,180 130,160
                       105,180 115,150 90,130 120,130" fill="#F39C12"/>
  </g>
</svg>)";
}

// ============================================================================
// SvgViewerWindow 实现
// ============================================================================

SvgViewerWindow::SvgViewerWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QtSvg 矢量图形基础示例");
    resize(900, 600);

    // ---- SVG 显示区域 ----
    svg_widget_ = new QSvgWidget(this);
    svg_widget_->setMinimumSize(300, 300);

    auto *scroll_area = new QScrollArea(this);
    scroll_area->setWidget(svg_widget_);
    scroll_area->setWidgetResizable(false);

    // ---- 右侧控制面板 ----
    auto *panel = new QWidget(this);
    auto *panel_layout = new QVBoxLayout(panel);

    // 打开文件按钮
    auto *open_button = new QPushButton("打开 SVG 文件", this);
    panel_layout->addWidget(open_button);

    // 加载演示 SVG
    auto *demo_button = new QPushButton("加载演示 SVG", this);
    panel_layout->addWidget(demo_button);

    // 缩放控制
    auto *zoom_label = new QLabel("缩放:", this);
    zoom_slider_ = new QSlider(Qt::Horizontal, this);
    zoom_slider_->setRange(10, 500);
    zoom_slider_->setValue(100);
    zoom_value_label_ = new QLabel("100%", this);
    auto *zoom_layout = new QHBoxLayout();
    zoom_layout->addWidget(zoom_label);
    zoom_layout->addWidget(zoom_slider_, 1);
    zoom_layout->addWidget(zoom_value_label_);
    panel_layout->addLayout(zoom_layout);

    // 着色控制
    auto *color_label = new QLabel("SVG 着色:", this);
    color_preview_ = new QLabel(this);
    color_preview_->setFixedSize(32, 32);
    color_preview_->setFrameShape(QFrame::Box);
    updateColorPreview(QColor("#3498DB"));

    auto *color_button = new QPushButton("选择颜色", this);
    auto *apply_color_button = new QPushButton("应用着色", this);
    auto *color_layout = new QHBoxLayout();
    color_layout->addWidget(color_label);
    color_layout->addWidget(color_preview_);
    color_layout->addWidget(color_button);
    color_layout->addWidget(apply_color_button);
    panel_layout->addLayout(color_layout);

    // 元素列表
    auto *elem_label = new QLabel("SVG 元素 (id):", this);
    element_list_ = new QLabel("(加载 SVG 后显示)", this);
    element_list_->setWordWrap(true);
    element_list_->setFrameShape(QFrame::StyledPanel);
    panel_layout->addWidget(elem_label);
    panel_layout->addWidget(element_list_);

    // 元素提取：渲染单个元素到图标
    auto *extract_button = new QPushButton("提取选中元素为图标", this);
    extracted_icon_label_ = new QLabel("(提取后显示)", this);
    extracted_icon_label_->setAlignment(Qt::AlignCenter);
    extracted_icon_label_->setFrameShape(QFrame::StyledPanel);
    extracted_icon_label_->setMinimumHeight(80);
    panel_layout->addWidget(extract_button);
    panel_layout->addWidget(extracted_icon_label_);

    // 状态标签
    status_label_ = new QLabel("就绪", this);
    panel_layout->addWidget(status_label_);

    panel_layout->addStretch();

    // ---- 主布局 ----
    auto *central = new QWidget(this);
    auto *main_layout = new QHBoxLayout(central);
    main_layout->addWidget(scroll_area, 1);
    main_layout->addWidget(panel, 0);
    setCentralWidget(central);

    // ---- 加载演示 SVG ----
    loadDemoSvg();

    // ---- 信号槽连接 ----

    // 打开文件
    connect(open_button, &QPushButton::clicked, this, [this]() {
        QString file = QFileDialog::getOpenFileName(
            this, "选择 SVG 文件", {},
            "SVG Files (*.svg);;All Files (*)");
        if (!file.isEmpty()) {
            loadSvgFromFile(file);
        }
    });

    // 加载演示
    connect(demo_button, &QPushButton::clicked, this,
            &SvgViewerWindow::loadDemoSvg);

    // 缩放
    connect(zoom_slider_, &QSlider::valueChanged, this,
            [this](int value) {
                zoom_value_label_->setText(QString("%1%").arg(value));
                applyZoom(value);
            });

    // 选择颜色
    connect(color_button, &QPushButton::clicked, this, [this]() {
        QColor color = QColorDialog::getColor(current_color_, this, "选择 SVG 填充色");
        if (color.isValid()) {
            current_color_ = color;
            updateColorPreview(color);
        }
    });

    // 应用着色
    connect(apply_color_button, &QPushButton::clicked, this, [this]() {
        if (current_svg_data_.isEmpty()) {
            status_label_->setText("请先加载 SVG 文件");
            return;
        }
        QByteArray colored = recolorSvg(current_svg_data_, current_color_);
        svg_widget_->load(colored);
        status_label_->setText("已应用着色: " + current_color_.name());
        applyZoom(zoom_slider_->value());
    });

    // 提取元素
    connect(extract_button, &QPushButton::clicked, this, [this]() {
        if (current_element_ids_.isEmpty()) {
            status_label_->setText("没有可提取的 SVG 元素");
            return;
        }
        // 提取第一个元素作为演示
        QString elemId = current_element_ids_.first();
        QSvgRenderer renderer(current_svg_data_);
        if (renderer.elementExists(elemId)) {
            QPixmap pixmap = renderSvgToPixmap(renderer, 64);
            extracted_icon_label_->setPixmap(pixmap);
            status_label_->setText("已提取元素: " + elemId);
        } else {
            status_label_->setText("元素不存在: " + elemId);
        }
    });
}

void SvgViewerWindow::loadSvgFromFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        status_label_->setText("无法打开文件: " + path);
        return;
    }
    current_svg_data_ = file.readAll();
    svg_widget_->load(current_svg_data_);
    base_size_ = svg_widget_->sizeHint();
    updateElementList();
    applyZoom(zoom_slider_->value());
    status_label_->setText("已加载: " + QFileInfo(path).fileName());
}

void SvgViewerWindow::loadDemoSvg()
{
    current_svg_data_ = createDemoSvg();
    svg_widget_->load(current_svg_data_);
    base_size_ = svg_widget_->sizeHint();
    updateElementList();
    applyZoom(zoom_slider_->value());
    status_label_->setText("已加载演示 SVG");
}

void SvgViewerWindow::updateElementList()
{
    current_element_ids_ = extractElementIds(current_svg_data_);
    if (current_element_ids_.isEmpty()) {
        element_list_->setText("(无带 id 的元素)");
    } else {
        element_list_->setText(current_element_ids_.join("\n"));
    }
}

void SvgViewerWindow::applyZoom(int percent)
{
    if (base_size_.isEmpty()) return;
    double scale = percent / 100.0;
    int w = static_cast<int>(base_size_.width() * scale);
    int h = static_cast<int>(base_size_.height() * scale);
    svg_widget_->setFixedSize(
        std::max(w, 50), std::max(h, 50));
}

void SvgViewerWindow::updateColorPreview(const QColor &color)
{
    QPixmap px(32, 32);
    px.fill(color);
    color_preview_->setPixmap(px);
}
