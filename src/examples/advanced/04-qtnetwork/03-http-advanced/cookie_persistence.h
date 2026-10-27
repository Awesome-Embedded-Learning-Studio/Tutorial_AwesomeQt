/// @file    cookie_persistence.h
/// @brief   持久化 Cookie Jar，将 Cookie 保存到本地 JSON 文件，应用重启后自动恢复。
///
/// @details 对应教程：进阶层 04-QtNetwork/03-HTTP 高级用法。
///          演示如何继承 QNetworkCookieJar 并结合 QSettings 实现跨会话的
///          Cookie 持久化存储。

#pragma once

#include <QList>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QString>

/// @brief 支持 JSON 文件持久化的 Cookie Jar。
/// @note  继承 QNetworkCookieJar 后重写 cookiesForUrl / setCookiesFromUrl，
///        在基类行为之上叠加磁盘读写。QSettings 提供跨平台的存储路径解析。
class PersistentCookieJar : public QNetworkCookieJar
{
    Q_OBJECT

public:
    /// @brief 构造函数，从磁盘加载已保存的 Cookie。
    /// @param[in] filePath Cookie 存储文件的完整路径。为空时使用 QSettings 默认路径。
    /// @param[in] parent   父对象指针。
    /// @note 若文件不存在或内容损坏，将创建空存储，不会抛出异常。
    explicit PersistentCookieJar(const QString& filePath = QString(),
                                 QObject* parent = nullptr);

    /// @brief 析构函数，自动将当前所有 Cookie 保存到磁盘。
    ~PersistentCookieJar() override;

    // 禁止拷贝和移动
    PersistentCookieJar(const PersistentCookieJar&) = delete;
    PersistentCookieJar& operator=(const PersistentCookieJar&) = delete;
    PersistentCookieJar(PersistentCookieJar&&) = delete;
    PersistentCookieJar& operator=(PersistentCookieJar&&) = delete;

    /// @brief 获取指定 URL 关联的所有 Cookie（重写）。
    /// @param[in] url 目标 URL。
    /// @return 匹配的 Cookie 列表。
    /// @note 基类实现已处理域名、路径、安全性匹配规则，此处直接调用基类。
    QList<QNetworkCookie> cookiesForUrl(const QUrl& url) const override;

    /// @brief 为指定 URL 设置 Cookie 并持久化（重写）。
    /// @param[in] cookieList 待设置的 Cookie 列表。
    /// @param[in] url        目标 URL。
    /// @return 设置是否成功。
    /// @note 每次调用均会触发磁盘写入，确保 Cookie 立即持久化。
    bool setCookiesFromUrl(const QList<QNetworkCookie>& cookieList,
                           const QUrl& url) override;

    /// @brief 将当前 Jar 中所有 Cookie 强制写入磁盘。
    /// @note 正常流程下析构时会自动保存，此方法用于需要手动触发持久化的场景。
    void saveToDisk();

    /// @brief 返回 Jar 中所有 Cookie（包括已过期的）。
    /// @note QNetworkCookieJar::allCookies() 是 protected 方法，
    ///       此方法提供一个 public 访问入口，方便外部查询和调试。
    [[nodiscard]] QList<QNetworkCookie> getAllCookies() const;

    /// @brief 批量替换 Jar 中的所有 Cookie 并持久化。
    /// @param[in] cookies 新的 Cookie 列表，完全替换原有内容。
    /// @note 适用于需要从外部注入预设 Cookie 的场景。
    void setAllCookiesAndSave(const QList<QNetworkCookie>& cookies);

private:
    /// @brief 从磁盘加载 Cookie 到 Jar 中。
    /// @note 加载后使用 setAllCookies 批量注入基类数据结构。
    void loadFromDisk();

    /// @brief 将 Cookie 列表序列化为 JSON 格式的字节数组。
    /// @param[in] cookies 待序列化的 Cookie 列表。
    /// @return JSON 字节数组。
    [[nodiscard]] static QByteArray serializeCookies(
        const QList<QNetworkCookie>& cookies);

    /// @brief 从 JSON 字节数组反序列化出 Cookie 列表。
    /// @param[in] data JSON 格式的字节数据。
    /// @return 解析得到的 Cookie 列表。解析失败时返回空列表。
    [[nodiscard]] static QList<QNetworkCookie> deserializeCookies(
        const QByteArray& data);

    QString m_filePath;  ///< Cookie 存储文件路径
};
