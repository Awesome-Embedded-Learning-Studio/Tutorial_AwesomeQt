import DefaultTheme from 'vitepress/theme'
import { h } from 'vue'
import type { Theme } from 'vitepress'
import HomeRoadmap from './components/HomeRoadmap.vue'
import HomeMeta from './components/HomeMeta.vue'
import CardGrid from './components/CardGrid.vue'
import CardLink from './components/CardLink.vue'
import FontSizeSwitcher from './components/FontSizeSwitcher.vue'
import ScreenshotCarousel from './components/ScreenshotCarousel.vue'
import { setupMermaid } from './mermaid-client'
import './custom.css'

export default {
  extends: DefaultTheme,
  Layout() {
    return h(DefaultTheme.Layout, null, {
      'home-features-before': () => h(ScreenshotCarousel),
      'home-features-after': () => h(HomeRoadmap),
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
  },
} satisfies Theme
