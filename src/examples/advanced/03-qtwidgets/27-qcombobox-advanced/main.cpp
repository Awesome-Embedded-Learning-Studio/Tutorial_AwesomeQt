/// @file    main.cpp
/// @brief   QComboBox 进阶演示程序入口。
///
/// 构建一个 CustomComboPopup 配合 ComboItemDelegate 的多列数据选择器，
/// 展示 showPopup 覆写防截断与 QStandardItemModel 驱动的三列弹窗渲染。
///
/// 对应教程：进阶层 03-QtWidgets/27-QComboBox 进阶。

#include "combo_item_delegate.h"
#include "custom_combo_popup.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QStandardItemModel>
#include <QVBoxLayout>

/// @brief 构建 Model 并填充城市数据（名称 / 编码 / 地区三列）。
/// @param[in] parent 父对象。
/// @return 填充好的 QStandardItemModel 指针。
static QStandardItemModel* createCityModel(QObject* parent)
{
    auto* model = new QStandardItemModel(parent);
    model->setHorizontalHeaderLabels({QStringLiteral("城市"), QStringLiteral("编码"),
                                      QStringLiteral("地区")});

    // 城市数据：{名称, 编码, 地区}
    struct CityEntry
    {
        const char* name;
        const char* code;
        const char* region;
    };

    // clang-format off
    static const CityEntry kCities[] = {
        {"北京",   "BJ", "华北"},
        {"上海",   "SH", "华东"},
        {"广州",   "GZ", "华南"},
        {"深圳",   "SZ", "华南"},
        {"成都",   "CD", "西南"},
        {"杭州",   "HZ", "华东"},
        {"武汉",   "WH", "华中"},
        {"西安",   "XA", "西北"},
        {"南京",   "NJ", "华东"},
        {"重庆",   "CQ", "西南"},
        {"天津",   "TJ", "华北"},
        {"苏州",   "SU", "华东"},
        {"长沙",   "CS", "华中"},
        {"郑州",   "ZZ", "华中"},
        {"大连",   "DL", "东北"},
    };
    // clang-format on

    for (const auto& city : kCities) {
        QList<QStandardItem*> row;
        // QLatin1String 用于运行时 const char* 到 QString 的零拷贝构造
        row.append(new QStandardItem(QString::fromUtf8(city.name)));
        row.append(new QStandardItem(QLatin1String(city.code)));
        row.append(new QStandardItem(QString::fromUtf8(city.region)));
        model->appendRow(row);
    }

    return model;
}

// ─────────────────────────────────────────────────────────────────────────────
// main
// ─────────────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    auto* window = new QWidget;
    auto* layout = new QVBoxLayout(window);

    // 标题
    auto* title = new QLabel(QStringLiteral("QComboBox 进阶演示——多列数据选择器"));
    title->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 14px;"));
    layout->addWidget(title);

    // Model 驱动的 QComboBox
    auto* combo = new CustomComboPopup;
    auto* model = createCityModel(combo);
    combo->setModel(model);
    combo->setModelColumn(0);  // 显示列设为第 0 列（城市名称）

    // 安装自定义 delegate——弹窗中绘制名称/编码/地区三列
    combo->setItemDelegate(new ComboItemDelegate(combo));

    // 选择信息展示区
    auto* infoLayout = new QHBoxLayout;
    auto* infoLabel = new QLabel(QStringLiteral("选中后信息："));
    auto* infoText = new QLabel(QStringLiteral("（请从下拉列表中选择城市）"));
    infoLayout->addWidget(infoLabel);
    infoLayout->addWidget(infoText, 1);

    layout->addWidget(combo);
    layout->addLayout(infoLayout);
    layout->addStretch();

    // 选中项变化时，通过 model()->index() 显式获取各列数据
    // 而不是依赖 currentData 的隐含 modelColumn 语义
    QObject::connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                     [combo, infoText, model](int row) {
                         if (row < 0) {
                             infoText->setText(QStringLiteral("（无选择）"));
                             return;
                         }
                         QString name = model->index(row, 0).data().toString();
                         QString code = model->index(row, 1).data().toString();
                         QString region = model->index(row, 2).data().toString();
                         infoText->setText(
                             QStringLiteral("城市: %1 | 编码: %2 | 地区: %3")
                                 .arg(name, code, region));
                     });

    window->setWindowTitle(QStringLiteral("QComboBox Advanced Demo"));
    window->resize(500, 200);
    window->show();

    return app.exec();
}
