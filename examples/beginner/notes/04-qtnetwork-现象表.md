# 04-qtnetwork 代码样例现象表

> 以下为各示例运行时的预期现象描述，供逐个核实时对照。
> 所有示例均为控制台应用（QCoreApplication），输出通过 `qDebug()` 打印。

---

## 01-tcp-socket-beginner

**前提条件：** 无需外部依赖。服务端与客户端在同一进程内通过 `QHostAddress::LocalHost:12345` 通信。

**运行时序：**
1. `EchoServer` 构造时立即监听端口 12345
2. `TcpClient` 构造后调用 `connectToServer` 发起连接
3. 连接建立后客户端自动发送第一条消息 `"Hello from Qt TCP Client!"`
4. 之后通过 `QTimer::singleShot` 按时序发送后续消息和断开连接

| 序号 | 演示要点 | 预期现象 |
|------|----------|----------|
| 1 | 服务端启动监听 | 打印 `=== Qt TCP Socket Basic Example ===` 随后打印 `[Server] Listening on port 12345 ...` |
| 2 | 客户端发起连接 | 打印 `[Client] Connecting to "127.0.0.1" : 12345 ...` |
| 3 | TCP 连接建立 | 客户端打印 `[Client] Connected to server!`；服务端打印 `[Server] Client connected: "127.0.0.1" :<peerPort> (total: 1)` |
| 4 | 客户端自动发送首条消息 | 客户端打印 `[Client] Sent: Hello from Qt TCP Client!`；服务端打印 `[Server] Received from "127.0.0.1" : Hello from Qt TCP Client!`；服务端回声后客户端打印 `[Client] Server replied: Hello from Qt TCP Client!` |
| 5 | 延迟 500ms 发送第二条消息 | 客户端打印 `[Client] Sent: This is the second message.`；服务端收到并回声；客户端打印 `[Client] Server replied: This is the second message.` |
| 6 | 延迟 1000ms 发送第三条消息 | 客户端打印 `[Client] Sent: Third message coming through.`；服务端收到并回声；客户端打印 `[Client] Server replied: Third message coming through.` |
| 7 | 延迟 2000ms 客户端主动断开 | 打印 `[Client] Disconnecting gracefully...`；随后客户端打印 `[Client] Disconnected from server.`；服务端打印 `[Server] Client disconnected: "127.0.0.1"` |
| 8 | 延迟 3000ms 打印总结并退出 | 打印 `=== Summary ===`，显示 `Server handled connections: 0 active`（客户端已断开），打印 `Demo finished.` 后程序退出 |

**补充说明：** 若服务器未启动或连接失败，客户端会触发指数退避重连（1s、2s、4s），最多重试 3 次后打印 `[Client] Max retries reached, giving up.` 并退出。

---

## 02-udp-socket-beginner

**前提条件：** 无需外部依赖。接收端和发送端在同一进程内通过 `QHostAddress::LocalHost:23456` 通信。

**运行时序：**
1. `UdpReceiver` 构造时绑定端口 23456
2. `UdpSender` 构造时绑定随机端口
3. 通过定时器按序发送单播、广播和多条消息

| 序号 | 演示要点 | 预期现象 |
|------|----------|----------|
| 1 | 启动接收端和发送端 | 打印 `=== Qt UDP Socket Basic Example ===`；接收端打印 `[Receiver] Listening on port 23456` |
| 2 | 延迟 500ms 单播消息 | 发送端打印 `[Sender] Unicast <N> bytes to "127.0.0.1" :23456 -> Hello via UDP unicast!`；接收端打印 `[Receiver] Got <N> bytes from "127.0.0.1" :<senderPort> -> Hello via UDP unicast!` |
| 3 | 延迟 1500ms 广播 DISCOVER 消息 | 发送端打印 `[Sender] Broadcast <N> bytes on port 23456 -> DISCOVER`；接收端打印收到 DISCOVER 消息后自动回复 `[Receiver] Replied with: HERE:<主机名>:<本机IP>`；发送端打印 `[Sender] Reply from "127.0.0.1" :<port> : HERE:<主机名>:<本机IP>` |
| 4 | 延迟 2500ms 连续发送 3 个独立数据报 | 发送端依次打印 3 条 `[Sender] Unicast ... -> Message #1/2/3`；随后打印 `[Sender] Sent 3 separate datagrams.` 和 `[Sender] Each should arrive as a complete message (UDP preserves datagram boundaries).`；接收端分别打印收到 3 条独立消息 |
| 5 | 延迟 4000ms 打印总结并退出 | 打印 `=== Summary ===`，说明 UDP 保持数据报边界、每次 writeDatagram 对应一次完整 readDatagram、但不保证送达，最后打印 `Demo finished.` 后退出 |

**补充说明：** UDP 无连接、不可靠，在本地回环环境下通常不会丢包，但在真实网络中可能出现丢失或乱序。

---

## 03-network-access-manager-beginner

**前提条件：** 需要互联网连接，访问 `httpbin.org` 公共测试服务。如果无法联网，每个请求都会打印网络错误信息。

**运行时序：**
1. 立即发起 GET 请求
2. 延迟 2000ms 发起 POST 请求
3. 延迟 4000ms 发起文件下载
4. 延迟 8000ms 打印总结退出

| 序号 | 演示要点 | 预期现象 |
|------|----------|----------|
| 1 | 创建 QNetworkAccessManager | 打印 `=== Qt QNetworkAccessManager Basic Example ===` |
| 2 | Demo 1：HTTP GET 请求 | 打印 `=== Demo 1: HTTP GET Request ===`；下载过程中打印进度 `[GET] Progress: XX% (N/M bytes)` 或 `[GET] Downloaded N bytes`；请求完成后打印 `[GET] HTTP Status: 200`、`[GET] Response size: <N> bytes`、`[GET] Response is valid JSON`；解析 JSON 后打印 `[GET] Server echoed User-Agent: QtNetworkTutorial/1.0` 和 `[GET] Server echoed X-Custom-Header: Qt-Network-Tutorial` |
| 3 | Demo 2：HTTP POST 请求（延迟 2s） | 打印 `=== Demo 2: HTTP POST Request ===` 和 `[POST] Sending JSON: {"name":"Qt Learner","topic":"QNetworkAccessManager","version":"6.9.1"}`；完成后打印 `[POST] HTTP Status: 200`，解析回显 JSON 打印 `name: Qt Learner`、`topic: QNetworkAccessManager`、`version: 6.9.1` 以及 `Content-Type sent: application/json` |
| 4 | Demo 3：带进度追踪的文件下载（延迟 4s） | 打印 `=== Demo 3: Download with Progress ===`；下载过程中打印 `[Download] XX% (X.X/X.X KB) @ X.X KB/s` 形式的进度信息；完成后打印 `[Download] HTTP Status: 200`、`[Download] Total size: <N> bytes (<KB>KB)` 和 `[Download] Content-Type: image/png` |
| 5 | 延迟 8s 打印总结并退出 | 打印 `=== Summary ===`，提示 QNetworkAccessManager 异步处理、需检查 error() 和 HTTP 状态码、记得 deleteLater()，最后打印 `Demo finished.` 后退出 |

**补充说明：** 所有请求均为 HTTPS，如果网络不通或 httpbin.org 不可达，会打印对应网络错误信息。整个 demo 运行约 8 秒。

---

## 04-websocket-beginner

**前提条件：** 无需外部依赖。WebSocket 服务端与客户端在同一进程内通过 `ws://localhost:12345` 通信。需要 QtWebSockets 模块。

**运行时序：**
1. 启动 `ChatServer` 监听端口 12345
2. 立即创建 client1 并连接
3. 延迟 500ms 创建 client2 并连接
4. 按时序发送文本、二进制、JSON 消息
5. 延迟 4000ms 和 4500ms 依次断开两个客户端

| 序号 | 演示要点 | 预期现象 |
|------|----------|----------|
| 1 | 启动 WebSocket 服务端 | 打印 `=== Qt WebSocket Basic Example ===`；服务端打印 `[Server] Listening on port ws://0.0.0.0:12345/...` |
| 2 | client1 连接 | client1 打印 `[Client] Connecting to ws://localhost:12345 ...`；连接后打印 `[Client] Connected to server!`；服务端打印 `[Server] Client connected: <addr>:<port> (total: 1)`；服务端广播 `[System] New client joined from <addr>:<port>`；client1 收到欢迎消息 `[Client] Text received (#1): [Server] Welcome to the chat room! Online: 1` |
| 3 | 延迟 500ms client2 连接 | client2 打印 `[Client] Connecting to ws://localhost:12345 ...` 和 `[Client] Connected to server!`；服务端打印 `[Server] Client connected: <addr>:<port> (total: 2)`；服务端广播 `[System] New client joined from <addr>:<port>`；client1 和 client2 均收到系统广播和各自的欢迎消息 |
| 4 | 延迟 1000ms client1 发送文本 | client1 打印 `[Client] Sent: Hello from Client 1!`；服务端打印 `[Server] Text from <addr> : Hello from Client 1!`；服务端向所有客户端广播此消息；client1 和 client2 均收到并打印 `[Client] Text received (#N): Hello from Client 1!` |
| 5 | 延迟 1500ms client2 发送文本 | client2 打印 `[Client] Sent: Hi there, this is Client 2!`；服务端打印收到消息并广播；两个客户端均收到 |
| 6 | 延迟 2000ms client1 发送二进制消息 | client1 打印 `[Client] Sent binary: 256 bytes`（0x00~0xFF 共 256 字节）；服务端打印 `[Server] Binary from <addr> : 256 bytes`；服务端向 client1 回复确认 `[Server] Received binary: 256 bytes`；client1 收到并打印 `[Client] Text received (#N): [Server] Received binary: 256 bytes` |
| 7 | 延迟 2500ms client2 发送 JSON 消息 | client2 打印 `[Client] Sent: {"type":"chat","from":"Client 2","content":"JSON message works!",...}`；服务端广播此 JSON 字符串；两个客户端均收到文本消息 |
| 8 | 延迟 4000ms client1 断开 | 打印 `[Client1] Disconnecting...` 和 `[Client] Disconnected.`；服务端打印 `[Server] Client disconnected` 并广播 `[System] Client left. Remaining: 1`；client2 收到此系统消息 |
| 9 | 延迟 4500ms client2 断开 | 打印 `[Client2] Disconnecting...` 和 `[Client] Disconnected.`；服务端打印断开并广播 `[System] Client left. Remaining: 0` |
| 10 | 延迟 5000ms 打印总结并退出 | 打印 `=== Summary ===`，显示 `Server remaining connections: 0`、`Client1 received messages: <N>`、`Client2 received messages: <N>`，最后打印 `Demo finished.` 后退出 |

**补充说明：** 心跳 ping 每 10 秒发送一次，由于 demo 运行仅 5 秒，不会触发 pong 相关输出。若运行时间延长到 10 秒以上，可以看到 `[Client] Pong received: <payload> (elapsed: <N>ms)` 输出。

---

## 05-ssl-tls-beginner

**前提条件：** 需要互联网连接，访问 `example.com:443`。需要系统安装 SSL 运行时库（Linux 需 OpenSSL，Windows 使用 Schannel）。若 SSL 不可用，Demo 1 会提示并在打印警告后直接返回。

**运行时序：**
1. 立即运行 Demo 1（SSL 支持检查）和 Demo 4（SSL 配置信息）
2. 立即运行 Demo 2（TLS 探测连接 example.com）
3. 延迟 2000ms 运行 Demo 3（HTTPS 请求）
4. 延迟 8000ms 打印总结退出

| 序号 | 演示要点 | 预期现象 |
|------|----------|----------|
| 1 | Demo 1：SSL 支持检查 | 打印 `=== Qt SSL/TLS Basic Example ===` 和 `=== Demo 1: SSL Support Check ===`；打印 `SSL supported: true`（若系统安装了 SSL 库）；打印 SSL 运行时版本号（如 `OpenSSL 3.x.x`）和编译时版本号。若不支持则打印警告并退出 |
| 2 | Demo 4：SSL 配置信息 | 打印 `=== Demo 4: SSL Configuration ===`；显示默认协议（如 `TlsV1_2OrLater`）；显示系统 CA 证书数量（通常数十到数百个）；显示支持的加密套件数量及首尾套件名称和位数；显示自定义配置协议 `TlsV1_2OrLater` |
| 3 | Demo 2：TLS 信息探测（连接 example.com:443） | 打印 `=== Demo 2: TLS Probe for example.com ===` 和 `Connecting to example.com : 443 ...`；连接成功后打印 `[OK] Encrypted connection established in <N>ms`；打印协商的协议（如 `TlsV1_3`）和加密套件（如 `TLS_AES_256_GCM_SHA384` 及位数）；打印服务器证书详情：Subject CN、Subject O、Issuer CN、有效期起止、序列号；若证书即将过期或已过期会打印 WARNING；可能先打印 SSL Errors（开发阶段自动忽略） |
| 4 | Demo 3：HTTPS 请求 example.com（延迟 2s） | 打印 `=== Demo 3: HTTPS Request to example.com ===`；TLS 连接后打印 `[OK] TLS connected, sending HTTP request...`；收到响应后打印 `--- HTTP Response Headers ---` 及 HTTP 响应头内容（如 `HTTP/1.1 200 OK`、`Content-Type`、`ETag` 等）；连接关闭后打印 `Connection closed.` |
| 5 | 延迟 8s 打印总结并退出 | 打印 `=== Summary ===`，提示 QSslSocket 对 QTcpSocket 透明加密、先检查 supportsSsl()、生产环境勿用 ignoreSslErrors()，最后打印 `Demo finished.` 后退出 |

**补充说明：** 如果出现 SSL 错误（例如证书链不完整），Demo 2 和 Demo 3 会打印 `[SSL Errors]` 列表，但代码中调用了 `ignoreSslErrors()` 继续连接（仅限开发演示）。整个 demo 运行约 8 秒。

---

## 06-serial-port-beginner

**前提条件：** Demo 1 和 Demo 2 依赖系统存在物理或虚拟串口设备（Linux: `/dev/ttyUSB*`/`/dev/ttyACM*`，Windows: `COM*`）。若无串口设备，Demo 1 会提示未找到端口，Demo 2 会展示配置 API 用法但不实际打开端口。Demo 3（帧解析）为纯内存模拟，不依赖任何硬件。Linux 用户需在 `dialout` 组中才能访问串口。

**运行时序：**
1. 立即运行 Demo 1（列出串口）
2. 立即运行 Demo 2（串口配置）
3. 立即运行 Demo 3（帧解析模拟）
4. 延迟 1000ms 打印总结退出

| 序号 | 演示要点 | 预期现象 |
|------|----------|----------|
| 1 | Demo 1：列出可用串口 | 打印 `=== Qt SerialPort Basic Example ===` 和 `=== Demo 1: Available Serial Ports ===`；若有串口设备，打印每个端口的 portName、systemLocation、description、manufacturer、Vendor ID、Product ID 以及支持的标准波特率范围；若无串口，打印 `No serial ports found on this system.` 并提示检查方法 |
| 2 | Demo 2：串口配置演示 | 打印 `=== Demo 2: Serial Port Configuration ===`；若无真实串口，展示 API 用法：打印 `Configured port: COM3`、`Baud rate: 115200`、`Data bits: 8`、`Parity: 0 (NoParity)`、`Stop bits: 1`、`Flow control: 0 (NoFlowControl)`；若有真实串口则尝试打开，成功打印 `Port opened successfully!` 及配置参数，失败打印错误原因 |
| 3 | Demo 3：帧解析模拟（不依赖硬件） | 打印 `=== Demo 3: Frame Parsing Simulation ===` 和帧格式说明 `Frame format: AA 55 [len] [data...] [xor_checksum]`；构造 3 个测试帧并打印十六进制内容（frame1=`aa 55 05 48 65 6c 6c 6f <checksum>` 即 "Hello"、frame2 为 "QtSerialPort"、frame3 为 5 字节二进制）；拼接后打印合并数据；逐帧解析打印 `[Frame 1] Valid - Payload: 48 65 6c 6c 6f ASCII: "Hello"`、`[Frame 2] Valid - Payload: <hex> ASCII: "QtSerialPort"`、`[Frame 3] Valid - Payload: 01 02 03 04 05`；最后打印 `Parsed 3 frame(s), 0 bytes remaining in buffer.` |
| 4 | 延迟 1s 打印总结并退出 | 打印 `=== Summary ===`，提示 QtSerialPort 使用与 QTcpSocket 相同的 readyRead/write 模式、需要在应用层实现帧解析，最后打印 `Demo finished.` 后退出 |

**补充说明：** Demo 3 的帧格式为自定义二进制协议：`0xAA 0x55` 帧头 + 1 字节数据长度 + N 字节数据 + 1 字节 XOR 校验和。该 demo 演示了如何将拼接在一起的多帧数据正确拆分和校验，是嵌入式串口通信中典型的帧解析模式。
