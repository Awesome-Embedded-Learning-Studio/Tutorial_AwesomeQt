---
id: "076"
title: "实现 P0 网络工具类应用（port-scanner/packet-analyzer等）"
category: code-library
priority: P1
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["032"]
blocks: []
estimated_effort: large
---

## 目标

实现 P0 网络工具类应用，包含 port-scanner（端口扫描器）、packet-analyzer（抓包分析器）、dns-lookup（DNS 查询工具）等。

## 验收标准

- [ ] 每个工具独立项目目录，含完整 CMake 构建文件
- [ ] 独立可编译运行
- [ ] 包含 `demo.gif` 演示动图
- [ ] 功能完整，可作为网络调试工具使用

## 实施说明

1. **port-scanner**：
   - 多线程端口扫描
   - 支持指定 IP 范围和端口范围
   - 实时进度显示
   - 扫描结果导出
2. **packet-analyzer**：
   - 使用 raw socket 或 libpcap 抓包
   - 解析常见协议（TCP/UDP/ICMP/HTTP）
   - 数据包列表 + 详情视图
   - 过滤和搜索功能
3. **dns-lookup**：
   - DNS 查询（A/AAAA/MX/CNAME/NS/TXT 记录）
   - 支持指定 DNS 服务器
   - 查询历史记录
   - 批量域名查询
4. 使用 Qt Network 模块实现，异步非阻塞设计

## 涉及文件

- `examples/network-tools/port-scanner/`（新建）
- `examples/network-tools/packet-analyzer/`（新建）
- `examples/network-tools/dns-lookup/`（新建）

## 参考资料

- Qt Network: https://doc.qt.io/qt-6/qtnetwork-index.html
- QTcpSocket: https://doc.qt.io/qt-6/qtcpsocket.html
- QDnsLookup: https://doc.qt.io/qt-6/qdnslookup.html
