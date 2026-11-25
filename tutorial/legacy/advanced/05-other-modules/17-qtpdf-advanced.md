---
title: "5.17 QtPdf 进阶：文本搜索、选中复制、书签导航"
description: "入门篇我们用 QPdfView 和 QPdfDocument 把 PDF 显示出来了，翻页也能跑了。但光能看还不够——真正的 PDF 阅读器需要全文搜索、文本选中复制、书签导航、缩略图列表。这篇把这些能力逐个拆开。"
---

# 现代Qt开发教程（进阶篇）5.17——QtPdf 进阶：文本搜索、选中复制、书签导航

## 1. 前言

入门篇我们把 QtPdf 的基础跑通了——`QPdfView` 显示 PDF，`QPdfDocument` 加载文档，翻页和缩放都能正常工作。但如果你真的用它来做阅读器，马上就会发现缺一堆东西：想搜索某个关键词定位到对应页，想选中一段文字复制出来，想通过书签快速跳转到某个章节，想有个缩略图列表方便在大文档中导航。

这些功能 QtPdf 模块全都提供了 API，只不过入门篇没展开。这篇我们逐个拆解：`QPdfSearchModel` 做全文搜索，`QPdfSelectionModel` 做文本选中与复制，`QPdfBookmarkModel` 做书签导航，最后再加一个缩略图列表的实现思路。

QtPdf 的底层是 PDFium——Chromium 项目中使用的 PDF 渲染引擎。Qt 把 PDFium 的能力封装成了 QML 和 C++ 双层 API。我们这篇主要讲 C++ API，因为搜索和选择的逻辑在 C++ 层面更清晰。

## 2. 环境说明

本文档基于 Qt 6.4+ 编写，需要 Qt6::PdfWidgets 模块（包含 `QPdfView`）。纯 QML 项目可以用 Qt6::Pdf 模块。CMake 中通过 `find_package(Qt6 REQUIRED COMPONENTS Pdf PdfWidgets)` 引入。注意 Pdf 模块在 Qt 6 的某些配置下可能不默认安装，需要确认安装时勾选了 QtPdf 组件。

## 3. 核心概念讲解

### 3.1 全文搜索与高亮定位——QPdfSearchModel

`QPdfSearchModel` 是一个标准的 Qt Model 类，它接管了 PDF 文档的全文搜索功能。你给它设定搜索关键词，它会在整个文档中搜索匹配项，以 Model 的形式暴露结果。

```cpp
/// @brief 初始化搜索模型并连接到文档。
/// @param[in] document PDF 文档对象指针。
/// @note QPdfSearchModel 需要一个有效的 QPdfDocument 才能工作。
void setup_search_model(QPdfDocument* document)
{
    auto* search_model = new QPdfSearchModel(this);
    search_model->setDocument(document);

    // 设置搜索关键词
    search_model->search("Qt Quick 3D");

    // 搜索结果通过 Model 的行来访问
    // 每行包含：text（匹配文本）、pageIndex（所在页）、location（页面内坐标）、count（该行出现次数）
    connect(search_model, &QPdfSearchModel::countChanged, this, [search_model]() {
        const int kCount = search_model->rowCount();
        for (int i = 0; i < kCount; ++i) {
            QModelIndex kIdx = search_model->index(i, 0);
            int page = search_model->data(kIdx, QPdfSearchModel::PageRole).toInt();
            QRectF location = search_model->data(kIdx, QPdfSearchModel::LocationRole).toRectF();
            qDebug() << "匹配项" << i << "在页面" << page << "位置" << location;
        }
    });
}
```

`QPdfSearchModel` 的搜索是异步的。调用 `search()` 后，结果不会立即可用，需要等 `countChanged` 信号发射后才能读取。搜索结果中的 `LocationRole` 返回的是匹配文本在页面坐标系中的矩形区域——这个矩形可以直接用来在 `QPdfView` 上绘制高亮。

在 `QPdfView` 上高亮搜索结果的做法是：继承 `QPdfView`，重写 `paintEvent`，在父类绘制完成后，用 `QPainter` 在匹配位置绘制半透明的矩形覆盖。当然更简单的方式是用 `QPdfView` 自带的 `QPdfPageNavigator` 跳转到对应页面，然后通过覆盖一层透明 Widget 来绘制高亮。

```cpp
/// @brief 跳转到指定搜索结果。
/// @param[in] search_model 搜索模型。
/// @param[in] row 搜索结果行号。
/// @param[in] view PDF 视图。
void jump_to_search_result(QPdfSearchModel* model, int row, QPdfView* view)
{
    QModelIndex kIdx = model->index(row, 0);
    int page = model->data(kIdx, QPdfSearchModel::PageRole).toInt();
    view->pageNavigator()->jump(page, QPointF{}, QPointF{});
}
```

搜索结果支持前向/后向遍历——常见的阅读器中「上一个/下一个」按钮就是这么实现的：维护一个当前结果索引，点上一个就减一，点下一个就加一，循环即可。

### 3.2 文本选中与复制——QPdfSelectionModel

`QPdfSelectionModel` 管理 PDF 页面中的文本选中状态。它的工作方式和 `QItemSelectionModel` 类似——你设置选中区域，它维护选中状态，并提供获取选中文本的方法。

不过在实际使用中，更直接的方式是使用 `QPdfDocument::getAllText()` 和 `QPdfDocument::getText()` 来获取页面文本，然后自行管理选中逻辑。`QPdfSelectionModel` 在 Qt 6.5+ 才引入，API 还在演化中。

```cpp
/// @brief 获取指定页面的全部文本内容。
/// @param[in] document PDF 文档。
/// @param[in] page 页码（从 0 开始）。
/// @return 页面文本字符串。
QString get_page_text(QPdfDocument* document, int page)
{
    return document->getAllText(page).text();
}

/// @brief 将选中文本复制到剪贴板。
/// @param[in] text 要复制的文本。
void copy_to_clipboard(const QString& text)
{
    QClipboard* kClipboard = QGuiApplication::clipboard();
    kClipboard->setText(text);
}
```

文本选中在视觉层面的实现需要你自己处理鼠标事件。当用户在 PDF 页面上拖动鼠标时，你需要把鼠标坐标转换为页面坐标，然后用 `QPdfDocument::getText(page, rect)` 获取该矩形区域内的文本。这个 `rect` 参数是页面坐标系中的矩形，`getText` 会返回该区域内所有文本字符及其位置。

一个常见的做法是在 `QPdfView` 上叠加一层透明的 Widget，拦截鼠标事件，根据拖动范围计算选中矩形，然后调用 `getText` 获取选中文本，同时绘制选中高亮。

现在有个调试题给大家思考：你调用了 `document->getAllText(0)` 但返回的文本是空的，可能是什么原因？

最可能的原因是 PDF 文件中的文字是以路径（Path）形式存储的，而不是以文本（Text）对象存储的。扫描版 PDF 就是这样——文字是扫描图像，没有可提取的文本。这种情况下 `getAllText` 会返回空字符串。解决方案是使用 OCR 工具先将扫描图像转为文本。

### 3.3 书签树导航——QPdfBookmarkModel

`QPdfBookmarkModel` 把 PDF 文档中的书签（也叫大纲/目录/Outline）暴露成一个树形 Model。大多数正式的 PDF 文档都有书签——它就是 PDF 左侧那个可展开的目录树。

```cpp
/// @brief 设置书签模型并连接到文档。
/// @param[in] document PDF 文档。
/// @param[in] tree_view 树形视图控件。
void setup_bookmark_model(QPdfDocument* document, QTreeView* tree_view)
{
    auto* bookmark_model = new QPdfBookmarkModel(this);
    bookmark_model->setDocument(document);

    tree_view->setModel(bookmark_model);
    tree_view->setHeaderHidden(true);

    // 点击书签跳转到对应页面
    connect(tree_view, &QTreeView::clicked, this,
        [bookmark_model, document](const QModelIndex& index) {
            int page = bookmark_model->data(index, QPdfBookmarkModel::PageRole).toInt();
            QPointF location = bookmark_model->data(index, QPdfBookmarkModel::LocationRole).toPointF();
            // 跳转到书签指向的页面和位置
            document->pageNavigator()->jump(page, location);
        });
}
```

`QPdfBookmarkModel` 是 `QAbstractItemModel` 的子类，天然支持树形展示。`PageRole` 返回书签指向的页码，`LocationRole` 返回页面内的坐标位置。把它直接喂给 `QTreeView` 就能得到书签树。

书签模型的加载也是异步的——`setDocument` 之后不会立刻有数据，需要等 `document` 的 `statusChanged` 信号表示文档加载完成后，书签模型才会填充数据。

### 3.4 页面缩略图列表

缩略图列表是 PDF 阅读器的标配功能。实现思路很直接：对每个页面调用 `QPdfDocument::render()` 获取一个缩小版的 `QImage`，然后用 `QListView` + 自定义 Delegate 展示。

```cpp
/// @brief 渲染指定页面的缩略图。
/// @param[in] document PDF 文档。
/// @param[in] page 页码。
/// @param[in] thumbnail_width 缩略图宽度（像素）。
/// @return 缩略图 QImage。
QImage render_thumbnail(QPdfDocument* document, int page, int thumbnail_width)
{
    // 获取页面原始尺寸
    QSizeF kPageSize = document->pagePointSize(page);

    // 计算缩放比例
    double kScale = thumbnail_width / kPageSize.width();
    QSize kRenderSize = (kPageSize * kScale).toSize();

    // 渲染页面为 QImage
    return document->render(page, kRenderSize);
}
```

缩略图渲染是比较耗时的操作（每个页面需要 PDFium 做一次完整的页面渲染），不能在主线程上对几百页的文档一次性全部渲染。正确做法是用后台线程逐页渲染，每渲染好一页就通过信号通知 UI 更新。可以用 `QThread` + `QObject::moveToThread` 或者 `QtConcurrent::run` 来实现。渲染好的缩略图用 `QPixmapCache` 缓存起来，避免重复渲染。

缩略图列表的 Model 可以继承 `QAbstractListModel`，在 `data()` 中根据 `Qt::DecorationRole` 返回对应页面的缩略图 `QPixmap`。`QListView` 配合 `setIconSize()` 设置合适的缩略图大小就行。

## 4. 踩坑预防

第一个坑是 `QPdfDocument::load()` 是异步操作，紧接着调用 `pagePointSize()` 或 `getAllText()` 会返回无效数据。很多人在 `load()` 之后马上就开始渲染缩略图或获取文本，结果拿到全是空的。解决方案是必须等 `statusChanged` 信号中 `status()` 变为 `QPdfDocument::Ready` 之后才操作文档。加载失败时状态会变为 `QPdfDocument::Error`，也需要处理。

第二个坑是 `QPdfSearchModel` 的搜索关键词是大小写不敏感的，但某些版本可能表现为大小写敏感。如果你发现搜索结果不完整，先确认是不是大小写问题。另外搜索结果中的 `LocationRole` 返回的矩形坐标是页面坐标系（以点为单位，72 DPI），不是像素坐标。如果你需要把它转换成屏幕坐标来绘制高亮，需要根据当前的缩放级别做换算。忘记换算的话高亮框的位置会错位。

第三个坑是缩略图渲染的内存占用。每个页面的缩略图虽然不大，但一个 500 页的文档如果全部缓存到内存，也是几十 MB 的 `QPixmap`。建议使用 `QPixmapCache` 设置一个合理的缓存上限（比如 50 MB），让 Qt 自动淘汰不常用的缩略图。同时不要在 Model 的 `data()` 中同步渲染缩略图——`data()` 在主线程执行，渲染一页可能需要几十毫秒，滚动的时侯会明显卡顿。

## 5. 练习项目

练习项目是一个功能较为完整的 PDF 阅读器。我们要基于 `QPdfView` 构建一个包含搜索面板、书签树、缩略图列表三个导航组件的阅读器。

搜索面板在窗口右侧或底部，包含一个搜索输入框和「上一个/下一个」按钮，搜索结果在 PDF 页面上以黄色半透明矩形高亮显示，当前选中项以不同颜色标注。书签树在左侧面板，用 `QTreeView` 展示文档的目录结构，点击书签跳转到对应页面。缩略图列表在左侧面板的书签树下方（或通过 Tab 切换），用 `QListView` 横向排列每页缩略图，点击缩略图跳转到对应页面，当前页面在缩略图列表中以蓝色边框标注。

完成标准是三个导航组件都能正确工作、互不冲突，跳转定位准确，缩略图渲染不卡顿主线程，搜索高亮位置准确，整体使用流畅。不需要实现文本选中复制（这是额外挑战）。

提示几个关键点：搜索高亮可以在 `QPdfView` 上叠加一层透明 Widget 来绘制；缩略图用 `QtConcurrent::run` 后台渲染；三个面板用 `QSplitter` 组织布局。

## 6. 官方文档参考链接

[Qt 文档 · QPdfDocument](https://doc.qt.io/qt-6/qpdfdocument.html) -- PDF 文档加载、渲染、文本提取

[Qt 文档 · QPdfSearchModel](https://doc.qt.io/qt-6/qpdfsearchmodel.html) -- 全文搜索模型

[Qt 文档 · QPdfBookmarkModel](https://doc.qt.io/qt-6/qpdfbookmarkmodel.html) -- 书签树模型

[Qt 文档 · QPdfView](https://doc.qt.io/qt-6/qpdfview.html) -- PDF 视图控件

[Qt 文档 · QPdfPageNavigator](https://doc.qt.io/qt-6/qpdfpagenavigator.html) -- 页面导航（跳转、前进、后退）

---

到这里 QtPdf 的进阶功能就拆完了。搜索、选中、书签、缩略图——这四个能力组合起来，足以构建一个专业级别的 PDF 阅读器。如果你的需求更复杂（比如 PDF 标注、表单填写），那需要考虑使用 QtWebEngine 配合 PDF.js，或者直接用 Poppler 库，QtPdf 目前还不支持这些高级功能。
