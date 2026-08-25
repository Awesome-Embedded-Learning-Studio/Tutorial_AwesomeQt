import DefaultTheme from 'vitepress/theme'
import { h } from 'vue'
import type { Theme } from 'vitepress'
import AppTitleBar from './components/ide/AppTitleBar.vue'
import StatusBar from './components/ide/StatusBar.vue'
import WelcomeHero from './components/home/WelcomeHero.vue'
import LayerCards from './components/home/LayerCards.vue'
import ExamplesGrid from './components/home/ExamplesGrid.vue'
import CompileOutput from './components/home/CompileOutput.vue'
import AelsStrip from './components/home/AelsStrip.vue'
import CardGrid from './components/CardGrid.vue'
import CardLink from './components/CardLink.vue'
import FontSizeSwitcher from './components/FontSizeSwitcher.vue'
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
      // 工作台外壳：网站 = 一个 Qt 桌面应用。标题栏经 --vp-layout-top-height
      // 由 VitePress 原生机制全链避让；状态栏 fixed 钉底（.Layout 已留 26px）。
      'layout-top': () => [h(ReadingProgress), h(AppTitleBar), h(ResizableSidebar)],
      'layout-bottom': () => h(StatusBar),
      // 首页 = Welcome 欢迎屏（index.md 零 hero/features frontmatter，零覆盖战）：
      // 交互演示（signal→slot 真控件复刻）→ 三层卡 → 示例画廊 → 编译输出 → 生态行。
      'home-hero-before': () => h(WelcomeHero),
      'home-features-before': () => h(LayerCards),
      'home-features-after': () => [h(ExamplesGrid), h(CompileOutput), h(AelsStrip)],
      // 字号切换器：桌面顶栏右侧 + 移动端汉堡菜单展开后
      'nav-bar-content-after': () => h(FontSizeSwitcher),
      'nav-screen-content-after': () => h(FontSizeSwitcher),
    })
  },
  setup() {
    setupMermaid()
  },
  enhanceApp({ app }) {
    app.component('CardGrid', CardGrid)
    app.component('CardLink', CardLink)
    app.component('ModuleCard', ModuleCard)
    app.component('ChapterHero', ChapterHero)
  },
} satisfies Theme
