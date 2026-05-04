/// 通过 QSslSocket 发送 HTTPS 请求并读取响应头
#pragma once

#include <QString>

void demoHttpsRequest(const QString &hostname, const QString &path = "/");
