import DefaultTheme from 'vitepress/theme'
import { h } from 'vue'
import type { Theme } from 'vitepress'
import HomeRoadmap from './components/HomeRoadmap.vue'
import HomeMeta from './components/HomeMeta.vue'
import './custom.css'

export default {
  extends: DefaultTheme,
  Layout() {
    return h(DefaultTheme.Layout, null, {
      'home-features-after': () => h(HomeRoadmap),
    })
  },
  enhanceApp({ app }) {
    app.component('HomeMeta', HomeMeta)
  },
} satisfies Theme
