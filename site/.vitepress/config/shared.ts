// 单一真相源：dev 配置 (config/index.ts) 与生产分卷构建 (scripts/build.ts) 共享这一份。
//
// 改 markdown 插件 / head / vite / vue / 主题基础项，**只改这一处**，三处自动同步。
// 历史教训：mermaid 曾因三处分别维护而漏改一处，dev 正常、生产构建废
// （见 MEMORY build-ts-drifts-from-main-config）。
//
// 不含 nav / sidebar —— 分卷构建配置不需要这俩，只有 dev 配置和根构建配置各自挂。

import type { MarkdownIt } from 'markdown-it'
import { cppTemplateEscapePlugin } from '../plugins/escape-cpp-templates'
import { mermaidPlugin } from '../plugins/mermaid-plugin'
import { viteCppEscape } from '../plugins/vite-escape-cpp'

/** 站点级基础字段：标题 / 语言 / base / vite / vue / head —— 三处配置完全一致 */
export const sharedBase = {
  title: 'AwesomeQt 教程',
  description: '系统化的现代 Qt 6 教程 — 从基础入门到源码解析',
  lang: 'zh-CN',
  base: '/Tutorial_AwesomeQt/',
  cleanUrls: true,
  lastUpdated: true,

  vite: {
    build: {
      chunkSizeWarningLimit: 5000,
    },
    plugins: [viteCppEscape()],
  },

  vue: {
    template: {
      compilerOptions: {
        isCustomElement: (tag: string) => tag.includes('-') || tag.includes('.'),
      },
    },
  },

  head: [
    ['link', { rel: 'icon', href: '/Tutorial_AwesomeQt/favicon.ico' }],
  ],
}

/** markdown 渲染配置：行号 / 主题 / 自定义插件。新增 markdown 插件只在这里 md.use(...) */
export const sharedMarkdown = {
  lineNumbers: true,
  theme: {
    light: 'github-light',
    dark: 'github-dark',
  },
  config(md: MarkdownIt) {
    cppTemplateEscapePlugin(md)
    md.use(mermaidPlugin)
  },
}

/** 主题基础项（不含 nav/sidebar，分卷构建不带这俩） */
export const sharedThemeBase = {
  search: {
    provider: 'local',
  },

  editLink: {
    pattern: 'https://github.com/Awesome-Embedded-Learning-Studio/Tutorial_AwesomeQt/edit/main/tutorial/:path',
    text: '在 GitHub 上编辑此页',
  },

  footer: {
    message: '基于 VitePress 构建',
    copyright: 'Copyright 2025-2026 Charliechen',
  },

  socialLinks: [
    { icon: 'github', link: 'https://github.com/Awesome-Embedded-Learning-Studio/Tutorial_AwesomeQt' },
  ],
}
