import type { DefaultTheme } from 'vitepress'

export const navZh: DefaultTheme.NavItem[] = [
  { text: '首页', link: '/' },
  {
    text: '入门教程',
    items: [
      { text: '00 · 环境搭建', link: '/beginner/00-environment-setup/' },
      { text: '01 · QtBase 核心模块', link: '/beginner/01-qtbase/' },
      { text: '02 · QtGui 绘图与图像', link: '/beginner/02-qtgui/' },
      { text: '03 · QtWidgets 传统界面', link: '/beginner/03-qtwidgets/' },
      { text: '04 · QtNetwork 网络编程', link: '/beginner/04-qtnetwork/' },
      { text: '05 · 其他扩展模块', link: '/beginner/05-other-modules/' },
      { text: '06 · QML 现代界面', link: '/beginner/06-qml/' },
    ],
  },
  {
    text: '进阶教程',
    items: [
      { text: '01 · QtBase 核心模块', link: '/advanced/01-qtbase/' },
      { text: '02 · QtGui 绘图与图像', link: '/advanced/02-qtgui/' },
      { text: '03 · QtWidgets 传统界面', link: '/advanced/03-qtwidgets/' },
      { text: '04 · QtNetwork 网络编程', link: '/advanced/04-qtnetwork/' },
      { text: '05 · 其他扩展模块', link: '/advanced/05-other-modules/' },
      { text: '06 · QML 现代界面', link: '/advanced/06-qml/' },
    ],
  },
  { text: '项目实战', link: '/engineering/' },
]
