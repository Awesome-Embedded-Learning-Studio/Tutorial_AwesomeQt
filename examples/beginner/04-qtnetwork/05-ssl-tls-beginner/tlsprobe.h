/// 连接到 HTTPS 服务器并打印证书和协议信息
#pragma once

#include <QString>

void probeTlsInfo(const QString &hostname, quint16 port = 443);
