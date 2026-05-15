> 以下进阶层 02-qtgui 文章已全部完成，归档于 (2026-05-10)

## 02 · QtGui（6 篇）

- [x] 🔴 `01-qpainter-advanced.md` — QPainter 进阶：双缓冲、合成模式、抗锯齿 ✅ 完成于 2026-05-10
  - `setRenderHint(QPainter::Antialiasing)` 开启抗锯齿
  - `QPixmap` 离屏缓冲消除闪烁（双缓冲原理）
  - `setCompositionMode` 图层合成模式（正片叠底/滤色/叠加）
  - `QPainterPath` 复杂路径绘制与裁剪

- [x] 🟡 `02-coordinate-transform-advanced.md` — 坐标变换进阶：矩阵组合与逆变换 ✅ 完成于 2026-05-10
  - `QTransform` 矩阵乘法组合多个变换
  - `QTransform::inverted()` 求逆变换（鼠标坐标映射回场景坐标）
  - 仿射变换 vs 投影变换的区别
  - `QPainter::worldTransform()` 在复杂绘制中保存/恢复

- [x] 🔴 `03-image-processing-advanced.md` — 图像处理进阶：像素操作与格式转换 ✅ 完成于 2026-05-10
  - `QImage::pixel()` / `setPixel()` 逐像素操作
  - `QImage::Format` 格式转换与内存布局
  - `QImageReader` 支持的格式与流式加载大图
  - `Qt::SmoothTransformation` vs `FastTransformation` 缩放质量

- [x] 🟡 `04-font-text-advanced.md` — 字体进阶：富文本与 QTextDocument ✅ 完成于 2026-05-10
  - `QTextDocument` 完整文档模型（段落/表格/图片）
  - `QTextCursor` 程序化构建富文本内容
  - `QTextCharFormat` / `QTextBlockFormat` 格式控制
  - `QTextDocument::toHtml()` / `toMarkdown()` 格式导出

- [x] 🟡 `05-opengl-advanced.md` — OpenGL 进阶：着色器与 VAO/VBO ✅ 完成于 2026-05-10
  - `QOpenGLShaderProgram` 编译链接顶点/片段着色器
  - VAO / VBO 创建绑定与顶点属性布局
  - 纹理加载：`QOpenGLTexture` 封装
  - `QOpenGLFramebufferObject` 离屏渲染

- [x] 🟡 `06-drag-drop-advanced.md` — 拖放进阶：自定义 MIME 与跨应用拖放 ✅ 完成于 2026-05-10
  - 自定义 MIME 类型定义与序列化
  - `dropMimeData()` 在 Model/View 中支持拖放重排
  - 跨应用文件拖放（从 Explorer/Finder 拖文件到 Qt 应用）
  - `Qt::DropAction` 区分复制/移动/链接操作
