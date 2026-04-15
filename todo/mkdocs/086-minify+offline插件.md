---
id: 086
title: "minify + offline 插件"
category: mkdocs
priority: P2
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: []
blocks: []
estimated_effort: small
---

# minify + offline 插件

## 目标

添加 mkdocs-minify-plugin 和 mkdocs-offline-plugin，减小构建产物体积并支持 PWA 离线访问。

## 验收标准

- [ ] mkdocs-minify-plugin 已安装并配置
- [ ] 构建后 HTML/CSS/JS 体积减小
- [ ] mkdocs-offline-plugin 已安装并配置
- [ ] 支持 PWA 离线访问模式
- [ ] 离线模式下页面可正常浏览和搜索

## 实施说明

1. 安装依赖：
   ```bash
   pip install mkdocs-minify-plugin mkdocs-offline-plugin
   ```
2. 在 `mkdocs.yml` 的 `plugins` 中添加：
   ```yaml
   plugins:
     - minify:
         minify_html: true
         minify_js: true
         minify_css: true
     - offline
   ```
3. 验证构建产物体积变化
4. 测试离线访问功能

## 涉及文件

- `mkdocs.yml`
- `requirements.txt`（如存在）

## 参考资料

- [mkdocs-minify-plugin](https://github.com/byrnereese/mkdocs-minify-plugin)
- [mkdocs-offline-plugin](https://github.com/tcmetzger/mkdocs-offline-plugin)
- [MkDocs PWA 离线模式](https://squidfunk.github.io/mkdocs-material/setup/setting-up-navigation/#offline-usage)
