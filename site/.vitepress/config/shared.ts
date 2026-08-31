// 单一真相源：dev 配置 (config/index.ts) 与生产分卷构建 (scripts/build.ts) 共享这一份。
//
// 改 markdown 插件 / head / vite / vue / 主题基础项，**只改这一处**，三处自动同步。
// 历史教训：mermaid 曾因三处分别维护而漏改一处，dev 正常、生产构建废
// （见 MEMORY build-ts-drifts-from-main-config）。
//
// 不含 nav / sidebar —— 分卷构建配置不需要这俩，只有 dev 配置和根构建配置各自挂。

import type { MarkdownIt } from 'markdown-it'
import { cppTemplateEscapePlugin } from '../plugins/escape-cpp-templates'
import { codeFoldPlugin } from '../plugins/code-fold-plugin'
import { kbdPlugin } from '../plugins/kbd-plugin'
import { mermaidPlugin } from '../plugins/mermaid-plugin'
import { viteCppEscape } from '../plugins/vite-escape-cpp'
import { getBuildInfo } from './build-info'

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
    // 字号切换首屏防闪烁：Vue 挂载前先从 localStorage 读档位映射成 --aq-font-scale
    // 写到 documentElement，只缩正文不缩布局（issue #18，旧 data-font-size+zoom 方案已废弃）。
    // 与 theme/components/FontSizeSwitcher.vue 的 STORAGE_KEY('awesomeqt-font-size') 与 scale 映射一致。
    [
      'script',
      {},
      `(function(){var m={xxsmall:'0.88',small:'0.94',normal:'1',large:'1.08',xxlarge:'1.16'};try{var s=localStorage.getItem('awesomeqt-font-size');document.documentElement.style.setProperty('--aq-font-scale',m[s]||'1');}catch(e){}})()`,
    ],
    // 可拖拽抽屉侧栏首屏防闪：hydration 前从 localStorage 还原宽度 CSS 变量。
    // 272 只是近似值——挂载后 ResizableSidebar 会按当前卷最长条目自适应实测（未存过宽度时）。
    // 与 theme/components/ResizableSidebar.vue 的 CONF（key=vp-sidebar-width）一致。
    // （右大纲栏拖拽已砍，TOC 固定 256，不再注入 --vp-aside-width。）
    [
      'script',
      {},
      `(function(){try{var w=parseInt(localStorage.getItem('vp-sidebar-width'));if(!w||w<200||w>480){w=272;}document.documentElement.style.setProperty('--vp-sidebar-width',w+'px');}catch(e){}})()`,
    ],
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
    md.use(codeFoldPlugin) // 必须在 mermaid 之后：覆写 fence 要拿到 mermaid 改型后的完整链
    md.use(kbdPlugin)
  },
}

/** 主题基础项（不含 nav/sidebar，分卷构建不带这俩） */
export const sharedThemeBase = {
  search: {
    provider: 'local',
    options: {
      translations: {
        button: {
          buttonText: '搜索教程',
          buttonAriaLabel: '搜索教程',
        },
        modal: {
          noResultsText: '没有找到相关内容',
          resetButtonTitle: '清除查询',
          displayDetails: '显示详细列表',
          footer: {
            selectText: '选择',
            navigateText: '切换',
            closeText: '关闭',
          },
        },
      },
    },
  },

  editLink: {
    pattern: 'https://github.com/Awesome-Embedded-Learning-Studio/Tutorial_AwesomeQt/edit/main/tutorial/:path',
    text: '在 GitHub 上编辑此页',
  },

  footer: {
    message: (() => {
      const { version, sha, date } = getBuildInfo()
      return `AwesomeQt ${version} · ${sha} · ${date} · CHECKED-BY: CI`
    })(),
    copyright: 'Copyright 2025-2026 Charliechen',
  },

  socialLinks: [
    { icon: 'github', link: 'https://github.com/Awesome-Embedded-Learning-Studio/Tutorial_AwesomeQt' },
  ],
}
