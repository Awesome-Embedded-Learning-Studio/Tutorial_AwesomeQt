import { defineConfig } from 'vitepress'
import { navZh } from './nav'
import { buildSidebar } from './sidebar'
import { cppTemplateEscapePlugin } from '../plugins/escape-cpp-templates'
import { viteCppEscape } from '../plugins/vite-escape-cpp'

export default defineConfig({
  srcDir: '../tutorial',

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

  markdown: {
    lineNumbers: true,
    theme: {
      light: 'github-light',
      dark: 'github-dark',
    },
    config(md) {
      cppTemplateEscapePlugin(md)
    },
  },

  themeConfig: {
    nav: navZh,
    sidebar: buildSidebar(),

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
  },
})
