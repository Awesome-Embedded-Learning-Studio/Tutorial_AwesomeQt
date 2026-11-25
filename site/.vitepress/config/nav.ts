import type { DefaultTheme } from 'vitepress'

// 旧版教程归档在 tutorial/legacy/ 下（issue #35），网址统一带 /legacy/ 前缀。
// 新版教程的入口等目录规划定下来后再加。
export const navZh: DefaultTheme.NavItem[] = [
  { text: '首页', link: '/' },
  {
    text: '旧版教程',
    items: [
      { text: '总入口', link: '/legacy/' },
      { text: '入门教程', link: '/legacy/beginner/' },
      { text: '进阶教程', link: '/legacy/advanced/' },
      { text: '专家教程', link: '/legacy/expert/' },
      { text: '实例文档', link: '/legacy/engineering/' },
    ],
  },
]
