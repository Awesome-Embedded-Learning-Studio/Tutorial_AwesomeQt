import { defineConfig } from 'vitepress'
import { navZh } from './nav'
import { buildSidebar } from './sidebar'
import { sharedBase, sharedMarkdown, sharedThemeBase } from './shared'

export default defineConfig({
  srcDir: '../tutorial',

  ...sharedBase,

  markdown: sharedMarkdown,

  themeConfig: {
    ...sharedThemeBase,

    nav: navZh,
    sidebar: buildSidebar(),
  },
})
