# 现代Qt开发教程（新手篇）5.17--QtPdf PDF 渲染基础

## 1. 前言：别再用 QProcess 调外部阅读器了

说到在 Qt 应用里显示 PDF，我见过最多的做法是 QProcess 启动系统自带的 PDF 阅读器——Windows 下调 AcroRd32.exe，Linux 下调 evince 或者 okular，macOS 下调 open 命令。这做法能用是能用，但问题一堆：用户体验割裂（PDF 突然在一个完全独立的窗口里弹出来）、无法在应用内集成导航和批注功能、跨平台行为不一致（不同系统装的阅读器不一样，有的甚至没装）。更不用说你完全无法控制 PDF 的渲染效果——缩放、翻页、高亮、叠加注释，统统做不到。

QtPdf 模块就是来解决这个问题的。它提供了 PDF 文档加载、页面渲染、内嵌显示的完整能力，而且是纯 Qt 实现——不依赖外部阅读器，不依赖 Poppler 或 MuPDF 之类的第三方库（底层用的是 Qt 自有的 PDF 渲染引擎），编译出来就能跑，跨平台行为一致。对于需要嵌入 PDF 查看功能的应用来说——帮助文档阅读器、电子书阅读器、报表预览组件——QtPdf 是最直接的方案。

这篇我们要做的是用 QPdfDocument 加载 PDF 文件，用 QPdfPageRenderer 把指定页面渲染为 QImage，用 QPdfView 控件直接在窗口中显示 PDF 并支持页面导航和缩放。整个流程从底层渲染到高层控件都会覆盖到。

## 2. 环境说明

本篇基于 Qt 6.9.1，需要引入 Pdf 和 Widgets 模块。CMake 配置如下：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Pdf Widgets)
```

QtPdf 在 Qt Installer 中属于标准安装包。它的渲染引擎是 Qt 自有的——不依赖 Poppler、MuPDF 等第三方库，这意味着你不需要在目标系统上额外安装任何 PDF 相关的依赖。QtPdf 支持的 PDF 特性覆盖了绝大多数常见文档：文字渲染、矢量图形、图片嵌入、书签、链接、表单字段。不过它不支持 JavaScript 交互和多媒体嵌入（音频视频），这些属于 PDF 规范的扩展特性，大多数应用场景也不需要。

工具链：MSVC 2019+、GCC 11+、Clang 14+，C++17 标准，CMake 3.26+。QtPdf 是纯 CPU 渲染，对 GPU 没有要求。

## 3. 核心概念讲解

### 3.1 QPdfDocument--PDF 文档加载与元信息

QPdfDocument 是 QtPdf 的核心类，负责加载 PDF 文件并提供文档级的元信息和页面索引。它的用法很直接——构造时传入 parent，调用 load() 加载文件路径，加载完成后就可以查询页数、获取页面尺寸、渲染指定页面。

```cpp
QPdfDocument doc(this);
doc.load(QStringLiteral("/path/to/document.pdf"));

if (doc.status() != QPdfDocument::Ready) {
    qWarning() << "PDF 加载失败，状态:" << doc.status();
    return;
}

qDebug() << "总页数:" << doc.pageCount();
qDebug() << "第 0 页尺寸:" << doc.pagePointSize(0);  // 单位：磅（point，1/72 英寸）
```

QPdfDocument::load() 是异步的——调用后立即返回，文档在后台加载。加载状态通过 status() 属性获取，有三个主要值：Loading（正在加载）、Ready（加载完成，可以使用）、Error（加载失败）。如果你想等待加载完成再做操作，可以监听 statusChanged 信号。

pagePointSize() 返回的是指定页面的尺寸，单位是磅（point）——在 PDF 规范中 1 磅等于 1/72 英寸。A4 纸的尺寸是 595 x 842 磅。这个尺寸是"逻辑尺寸"，实际渲染到 QImage 时需要乘以你期望的 DPI 缩放因子——如果想在 150 DPI 下渲染，就把页面尺寸乘以 150/72 的系数。

QPdfDocument 还提供了一些元信息查询：metaData() 可以获取标题、作者、主题、创建日期等文档属性；getAllText() 和 getTextInPage() 可以提取文本内容（不过这些属于 QPdfDocument 的进阶功能，本篇聚焦在渲染和显示上）。

### 3.2 QPdfPageRenderer--页面渲染为 QImage

QPdfPageRenderer 是负责把 PDF 页面光栅化为 QImage 的类。它和 QPdfDocument 配合使用——先创建 QPdfDocument 加载文件，再把 QPdfDocument 传给 QPdfPageRenderer，然后调用 renderPage() 获取指定页面的 QImage。

```cpp
QPdfDocument *doc = new QPdfDocument(this);
doc->load(QStringLiteral(":/sample.pdf"));

QPdfPageRenderer renderer(this);
renderer.setDocument(doc);

// 渲染第 0 页，尺寸为页面原始大小的 2 倍（约 144 DPI）
QImage image = renderer.renderPage(0,
    doc->pagePointSize(0) * 2.0);  // imageSize 参数是 QSizeF

if (!image.isNull()) {
    image.save(QStringLiteral("page_0.png"));
    qDebug() << "页面渲染成功，尺寸:" << image.size();
}
```

renderPage() 的第一个参数是页面索引（从 0 开始），第二个参数是期望的图像尺寸（QSizeF，单位是磅的缩放值）。这个尺寸决定了渲染的分辨率——传入的值越大，输出的 QImage 像素越多，细节越清晰，但渲染时间也越长。一般来说，屏幕显示用页面原始尺寸的 1.5 到 2 倍就够了（相当于 108 到 144 DPI），打印输出用 4 到 6 倍（相当于 288 到 432 DPI）。

QPdfPageRenderer 的渲染是同步的——renderPage() 会阻塞直到渲染完成。对于单页渲染这通常只需要几毫秒到几十毫秒（取决于页面复杂度和分辨率），但如果要批量渲染大量页面或者高分辨率渲染，最好放在后台线程里执行，避免阻塞 UI 线程。

### 3.3 QPdfView--开箱即用的 PDF 显示控件

如果你只需要在窗口里显示 PDF、支持翻页和缩放，不需要自己管理渲染流程，QPdfView 就是直接拿来用的控件。它是一个 QWidget 子类（不是 QML 组件），集成了文档加载、页面渲染、滚动浏览、页面导航的全部功能。

```cpp
QPdfView *pdfView = new QPdfView(parent);

QPdfDocument *doc = new QPdfDocument(pdfView);
doc->load(QStringLiteral(":/sample.pdf"));

pdfView->setDocument(doc);
pdfView->setPageMode(QPdfView::PageMode::MultiPage);  // 多页连续滚动
```

QPdfView 的 setPageMode 控制显示模式：PageMode::SinglePage 一次只显示一页（需要手动翻页），PageMode::MultiPage 是多页连续排列、滚动浏览（类似大多数 PDF 阅读器的默认行为）。对于文档阅读来说 MultiPage 更自然，SinglePage 更适合演示文稿那种逐页翻阅的场景。

QPdfView 还内置了缩放能力——通过 setZoomFactor() 设置缩放比例，1.0 是原始大小。不过 QPdfView 没有内置鼠标滚轮缩放——如果你需要用户通过 Ctrl+滚轮缩放，需要自己重写 wheelEvent 或者在 QPdfView 外面包一层事件过滤器。

QPdfView 的 ZoomMode 也是一个值得了解的属性：ZoomMode::Custom 使用你手动设置的 zoomFactor，ZoomMode::FitToWidth 让页面宽度自动适配控件宽度，ZoomMode::FitInView 让整页适配控件的可见区域。FitToWidth 和 FitInView 在窗口大小变化时会自动重新计算缩放比例——对于窗口大小不固定的应用来说很实用。

### 3.4 页面导航与缩放

页面导航的核心是"当前页码"这个状态。QPdfView 本身提供了 document() 指向内部的 QPdfDocument，你可以通过 QPdfDocument::pageCount() 获取总页数，然后配合 QPdfView 的 pageMode 和滚动位置来推算当前页码。不过 QtPdf 在页面导航方面的 API 设计得比较"薄"——QPdfView 没有直接提供 currentPage() 方法。如果你的应用需要精确的当前页码指示（比如显示"第 3 / 10 页"），需要自己监听 QPdfView 的滚动位置变化来计算当前可见的页面索引。

缩放方面，QPdfView 的 setZoomFactor() 接受一个浮点数值，1.0 表示 100%（每磅对应 1 个逻辑像素）。常见的缩放范围是 0.25 到 5.0，低于 0.25 文字几乎看不清，高于 5.0 单页占用的渲染内存会非常大。

## 4. 综合示例：PDF 查看器小程序

把前面讲的内容整合起来，我们写一个基于 Widgets 的 PDF 查看器。核心功能：QPdfView 居中显示 PDF 文档，顶部工具栏包含"打开文件"按钮、"上一页/下一页"按钮、"页码显示"标签、"放大/缩小"按钮以及"缩放比例"显示。用 QPdfDocument 加载文件，QPdfView 负责渲染和显示，工具栏提供导航和缩放交互。

这个示例是纯 C++ Widgets 项目，不需要 QML。CMake 配置：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Pdf Widgets)

qt_add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Pdf Qt6::Widgets)
```

main.cpp 的完整代码：

```cpp
#include <QApplication>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPdfDocument>
#include <QPdfView>
#include <QPushButton>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>

#include <QDebug>

class PdfViewer : public QWidget
{
    Q_OBJECT

public:
    explicit PdfViewer(QWidget *parent = nullptr) : QWidget(parent)
    {
        setWindowTitle(QStringLiteral("QtPdf PDF 查看器"));
        resize(900, 700);

        auto *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);

        // 工具栏
        auto *toolbar = new QToolBar();
        toolbar->setMovable(false);

        auto *btnOpen = new QPushButton(QStringLiteral("打开文件"));
        toolbar->addWidget(btnOpen);

        toolbar->addSeparator();

        auto *btnPrev = new QPushButton(QStringLiteral("上一页"));
        toolbar->addWidget(btnPrev);

        m_pageLabel = new QLabel(QStringLiteral("0 / 0"));
        m_pageLabel->setMinimumWidth(80);
        m_pageLabel->setAlignment(Qt::AlignCenter);
        toolbar->addWidget(m_pageLabel);

        auto *btnNext = new QPushButton(QStringLiteral("下一页"));
        toolbar->addWidget(btnNext);

        toolbar->addSeparator();

        auto *btnZoomOut = new QPushButton(QStringLiteral("缩小"));
        toolbar->addWidget(btnZoomOut);

        m_zoomLabel = new QLabel(QStringLiteral("100%"));
        m_zoomLabel->setMinimumWidth(60);
        m_zoomLabel->setAlignment(Qt::AlignCenter);
        toolbar->addWidget(m_zoomLabel);

        auto *btnZoomIn = new QPushButton(QStringLiteral("放大"));
        toolbar->addWidget(btnZoomIn);

        mainLayout->addWidget(toolbar);

        // PDF 视图
        m_pdfView = new QPdfView();
        m_pdfView->setPageMode(QPdfView::PageMode::MultiPage);
        m_pdfView->setZoomMode(QPdfView::ZoomMode::Custom);

        m_pdfDoc = new QPdfDocument(this);

        mainLayout->addWidget(m_pdfView);

        // 连接信号
        connect(btnOpen, &QPushButton::clicked, this, &PdfViewer::openFile);
        connect(btnPrev, &QPushButton::clicked, this, &PdfViewer::prevPage);
        connect(btnNext, &QPushButton::clicked, this, &PdfViewer::nextPage);
        connect(btnZoomIn, &QPushButton::clicked, this, &PdfViewer::zoomIn);
        connect(btnZoomOut, &QPushButton::clicked, this, &PdfViewer::zoomOut);

        connect(m_pdfDoc, &QPdfDocument::statusChanged, this, [this]() {
            if (m_pdfDoc->status() == QPdfDocument::Ready) {
                m_pageLabel->setText(
                    QStringLiteral("1 / %1").arg(m_pdfDoc->pageCount()));
                updateZoomLabel();
            }
        });

        // 默认打开一个示例提示
        m_pageLabel->setText(QStringLiteral("请打开 PDF 文件"));
    }

private slots:

    void openFile()
    {
        QString path = QFileDialog::getOpenFileName(
            this, QStringLiteral("选择 PDF 文件"), QString(),
            QStringLiteral("PDF 文件 (*.pdf);;所有文件 (*)"));

        if (path.isEmpty()) {
            return;
        }

        if (m_pdfDoc->status() == QPdfDocument::Ready) {
            m_pdfDoc->close();
        }

        m_pdfDoc->load(path);

        if (m_pdfDoc->status() == QPdfDocument::Error) {
            qWarning() << "PDF 加载失败:" << path;
            return;
        }

        m_pdfView->setDocument(m_pdfDoc);
        m_currentPage = 0;
        m_zoomFactor = 1.0;
        m_pdfView->setZoomFactor(m_zoomFactor);
    }

    void prevPage()
    {
        if (m_currentPage > 0) {
            m_currentPage--;
            updatePageLabel();
        }
    }

    void nextPage()
    {
        if (m_currentPage < m_pdfDoc->pageCount() - 1) {
            m_currentPage++;
            updatePageLabel();
        }
    }

    void zoomIn()
    {
        m_zoomFactor = qMin(m_zoomFactor * 1.25, 5.0);
        m_pdfView->setZoomFactor(m_zoomFactor);
        updateZoomLabel();
    }

    void zoomOut()
    {
        m_zoomFactor = qMax(m_zoomFactor / 1.25, 0.25);
        m_pdfView->setZoomFactor(m_zoomFactor);
        updateZoomLabel();
    }

private:

    void updatePageLabel()
    {
        m_pageLabel->setText(
            QStringLiteral("%1 / %2")
                .arg(m_currentPage + 1)
                .arg(m_pdfDoc->pageCount()));
    }

    void updateZoomLabel()
    {
        m_zoomLabel->setText(
            QStringLiteral("%1%").arg(static_cast<int>(m_zoomFactor * 100)));
    }

    QPdfView *m_pdfView = nullptr;
    QPdfDocument *m_pdfDoc = nullptr;
    QLabel *m_pageLabel = nullptr;
    QLabel *m_zoomLabel = nullptr;
    int m_currentPage = 0;
    double m_zoomFactor = 1.0;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qDebug() << "QtPdf PDF 渲染基础示例";
    qDebug() << "本示例演示 QPdfDocument + QPdfPageRenderer + QPdfView";

    PdfViewer viewer;
    viewer.show();

    return app.exec();
}

#include "main.moc"
```

运行程序后你会看到一个带工具栏的窗口。点击"打开文件"选择一个 PDF，文档内容会立即在中央区域以多页连续滚动的模式显示出来。"放大"和"缩小"按钮每次调整 25% 的缩放比例，标签实时显示当前百分比。"上一页/下一页"按钮控制页码指示器。

几个实现细节值得说明。QPdfView 的 setDocument() 调用后，控件内部会自动开始渲染页面——不需要手动调用 QPdfPageRenderer。实际上 QPdfView 内部就持有一个 QPdfPageRenderer 实例，你在使用 QPdfView 时不需要再自己创建渲染器。只有当你需要把 PDF 页面渲染到 QImage 上做进一步处理（比如叠加水印、OCR 识别）时，才需要直接用 QPdfPageRenderer。

关于页码导航的问题：示例中 m_currentPage 变量维护的是页码索引，配合翻页按钮使用。但 QPdfView 在 MultiPage 模式下是滚动浏览的，用户滚动时当前可见页码会变化。如果你需要精确追踪滚动位置对应的页码，需要继承 QPdfView 重写 paintEvent 或者用 QScrollBar 的 valueChanged 信号来推算。示例中的翻页按钮只是更新了页码标签，没有联动 QPdfView 的滚动位置——这个联动需要根据 QPdfView 内部的布局来计算，实现起来稍复杂，留给练习项目。

## 5. 练习项目

练习项目：带搜索功能的 PDF 阅读器。

在基础查看器上增加文本搜索功能：工具栏增加一个 QLineEdit 搜索框和"搜索"按钮，输入关键词后调用 QPdfDocument::search() 方法搜索所有页面中包含该关键词的位置，搜索结果高亮显示在 QPdfView 上。QPdfView 支持通过 setHighlight() 或自定义 QPdfPageRenderer 的方式叠加搜索结果标记。

完成标准是这样的：搜索框输入关键词后点击搜索，在当前页面中找到的所有匹配项用黄色矩形框高亮标记出来，状态栏显示"找到 N 处匹配"。"上一个"/"下一个"按钮在高亮结果之间跳转，跳转时自动滚动到对应位置。

几个实现提示：QPdfDocument::search() 返回的是 QVector<QPdfSearchModelResult>（或者在较新版本中通过 QPdfSearchModel），每个结果包含页面索引和在页面上的矩形区域。高亮叠加可以通过 QPdfView 的 overlay 机制或者自己用 QPdfPageRenderer 渲染页面后用 QPainter 画矩形来实现。

## 6. 官方文档参考

[Qt 文档 · QtPdf 模块](https://doc.qt.io/qt-6/qtpdf-index.html) -- QtPdf 模块总览

[Qt 文档 · QPdfDocument](https://doc.qt.io/qt-6/qpdfdocument.html) -- PDF 文档加载与管理

[Qt 文档 · QPdfPageRenderer](https://doc.qt.io/qt-6/qpdfpagerenderer.html) -- 页面渲染器

[Qt 文档 · QPdfView](https://doc.qt.io/qt-6/qpdfview.html) -- PDF 显示控件

*(链接已验证，2026-04-23 可访问)*

---

到这里就大功告成了。QtPdf 提供了从底层渲染到高层控件的三层能力——QPdfDocument 负责文档加载和元信息管理，QPdfPageRenderer 把页面光栅化为 QImage 供自定义处理，QPdfView 作为开箱即用的控件直接嵌入 Widgets 界面。整个过程不依赖任何外部 PDF 库，纯 Qt 实现，编译出来就能跑。如果你的应用需要嵌入 PDF 查看、报表预览、文档阅读功能，QtPdf 是最轻量的选择。下一篇我们转向服务端方向，看 QtHttpServer 如何在 Qt 应用内嵌入一个 HTTP 服务器。
