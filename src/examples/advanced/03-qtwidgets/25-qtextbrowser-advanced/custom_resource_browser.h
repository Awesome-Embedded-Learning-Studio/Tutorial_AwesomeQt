/// @file    custom_resource_browser.h
/// @brief   CustomResourceBrowser 类声明——自定义协议资源加载与历史导航演示。
///
/// 对应教程：进阶层 03-QtWidgets/25-QTextBrowser 进阶。

#pragma once

#include <QImage>
#include <QMap>
#include <QTextBrowser>
#include <QUrl>

class QLabel;
class QPushButton;

/// 自定义协议资源加载与历史栈安全导航演示控件。
///
/// 展示三个核心知识点：
/// - 覆写 loadResource() 处理 "myapp://" 自定义协议的资源请求
/// - 安全的历史栈导航：backward/forward 配合 backwardHistoryCount 检查
/// - anchorClicked 信号与 setOpenLinks(false) 的完全手动导航控制
class CustomResourceBrowser : public QTextBrowser
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化模拟数据库、界面与信号槽。
    /// @param[in] parent 父控件指针。
    explicit CustomResourceBrowser(QWidget* parent = nullptr);

    /// @brief 创建导航工具栏控件（后退/前进/首页按钮 + 历史标签）。
    /// @return 包含导航控件的 QWidget 指针。
    /// @note 必须在构造后调用，返回的控件由调用者负责布局。
    QWidget* createNavigationToolBar();

private:
    /// @brief 初始化模拟图片数据库（QMap<QString, QImage>）。
    void initMockDatabase();

    /// @brief 构建第一页 HTML 内容（包含 myapp:// 协议图片和导航链接）。
    /// @return HTML 字符串。
    QString buildPageOne() const;

    /// @brief 构建第二页 HTML 内容（包含内部导航和外部链接）。
    /// @return HTML 字符串。
    QString buildPageTwo() const;

    /// @brief 更新导航按钮的启用状态和历史信息标签。
    void updateNavigationState();

protected:
    /// @brief 覆写 loadResource，拦截 "myapp://" 协议的图片请求。
    /// @param[in] type 资源类型（QTextDocument::ResourceType）。
    /// @param[in] name 资源 URL。
    /// @return 加载到的资源 QVariant，未命中则回退到基类实现。
    QVariant loadResource(int type, const QUrl& name) override;

private slots:
    /// @brief 安全后退：检查 backwardHistoryCount > 0 后调用 backward()。
    void safeBackward();

    /// @brief 安全前进：检查 forwardHistoryCount > 0 后调用 forward()。
    void safeForward();

    /// @brief 回到首页。
    void goHome();

    /// @brief 处理链接点击：区分内部页面、自定义协议和外部链接。
    /// @param[in] url 被点击的链接 URL。
    void handleAnchorClicked(const QUrl& url);

private:
    // --- 模拟数据库 ---
    QMap<QString, QImage> m_imageDatabase;  // 图片名 -> QImage 的模拟存储

    // --- 导航控件 ---
    QPushButton* m_btnBack;     // 后退按钮
    QPushButton* m_btnForward;  // 前进按钮
    QPushButton* m_btnHome;     // 首页按钮
    QLabel*      m_historyLabel;// 历史栈状态标签
};
