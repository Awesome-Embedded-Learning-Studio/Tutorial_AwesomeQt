import DefaultTheme from 'vitepress/theme'
import { h } from 'vue'
import type { Theme } from 'vitepress'
import HomeRoadmap from './components/HomeRoadmap.vue'
import HomeMeta from './components/HomeMeta.vue'
import CardGrid from './components/CardGrid.vue'
import CardLink from './components/CardLink.vue'
import { setupMermaid } from './mermaid-client'
import './custom.css'

export default {
  extends: DefaultTheme,
  Layout() {
    return h(DefaultTheme.Layout, null, {
      'home-features-after': () => h(HomeRoadmap),
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
