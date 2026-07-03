import DefaultTheme from 'vitepress/theme'
import { h } from 'vue'
import type { Theme } from 'vitepress'
import HomeRoadmap from './components/HomeRoadmap.vue'
import HomeMeta from './components/HomeMeta.vue'
import CardGrid from './components/CardGrid.vue'
import CardLink from './components/CardLink.vue'
import FontSizeSwitcher from './components/FontSizeSwitcher.vue'
import ScreenshotCarousel from './components/ScreenshotCarousel.vue'
import QtHero from './components/QtHero.vue'
import ResizableSidebar from './components/ResizableSidebar.vue'
import ReadingProgress from './components/ReadingProgress.vue'
import ModuleCard from './components/ModuleCard.vue'
import ChapterHero from './components/ChapterHero.vue'
import { setupMermaid } from './mermaid-client'
import './custom.css'

export default {
  extends: DefaultTheme,
  Layout() {
    return h(DefaultTheme.Layout, null, {
      // 自定义首页 Hero（替代默认 VPHero，读 frontmatter.hero）
      'home-hero-before': () => h(QtHero),
      'home-features-before': () => h(ScreenshotCarousel),
      'home-features-after': () => h(HomeRoadmap),
      // 顶部阅读进度条 + 可拖拽侧栏（左右栏宽度）
      'layout-top': () => [h(ReadingProgress), h(ResizableSidebar)],
      // 字号切换器：桌面顶栏右侧 + 移动端汉堡菜单展开后
      'nav-bar-content-after': () => h(FontSizeSwitcher),
      'nav-screen-content-after': () => h(FontSizeSwitcher),
    })
  },
  setup() {
    setupMermaid()
  },
  enhanceApp({ app }) {
    app.component('HomeMeta', HomeMeta)
    app.component('CardGrid', CardGrid)
    app.component('CardLink', CardLink)
    app.component('ModuleCard', ModuleCard)
    app.component('ChapterHero', ChapterHero)
  },
} satisfies Theme
