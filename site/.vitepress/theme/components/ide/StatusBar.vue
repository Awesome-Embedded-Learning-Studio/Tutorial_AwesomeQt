<script setup lang="ts">
// 状态栏：应用窗口的底条，全站每页可见（layout-bottom 插槽 + fixed 钉底，
// .Layout 已留 26px 让位）。右侧行号是 IDE 彩蛋：滚动位置映射成「文档行号」。
// 版本/ref 来自 home-data 快照字面量——build-info.ts 会 execFileSync 调 git，
// 客户端组件禁止 import（hydration 会炸）。
import { onMounted, onBeforeUnmount, ref } from 'vue'
import { statusBar, ACTIONS_URL } from '../home/home-data'

const line = ref<number | null>(null) // SSR/首帧渲染「--」防 hydration 不匹配
let ticking = false

function onScroll() {
  if (ticking) return
  ticking = true
  requestAnimationFrame(() => {
    const doc = document.documentElement
    const max = doc.scrollHeight - window.innerHeight
    const ratio = max > 0 ? window.scrollY / max : 0
    line.value = Math.max(1, Math.round(ratio * 1024))
    ticking = false
  })
}

onMounted(() => {
  onScroll()
  window.addEventListener('scroll', onScroll, { passive: true })
})

onBeforeUnmount(() => {
  window.removeEventListener('scroll', onScroll)
})
</script>

<template>
  <footer class="ide-statusbar" role="contentinfo">
    <span class="ide-statusbar__seg ide-statusbar__ready">
      <i class="ide-statusbar__dot" aria-hidden="true" />{{ statusBar.ready }}
    </span>
    <a class="ide-statusbar__seg" :href="ACTIONS_URL" target="_blank" rel="noopener">
      {{ statusBar.ci }}
    </a>
    <span class="ide-statusbar__seg ide-statusbar__units">{{ statusBar.units }}</span>
    <span class="ide-statusbar__seg ide-statusbar__qt">{{ statusBar.qt }}</span>
    <a class="ide-statusbar__seg" :href="statusBar.repo" target="_blank" rel="noopener">GitHub</a>
    <span class="ide-statusbar__right">
      <span class="ide-statusbar__line">行 {{ line ?? '--' }}</span>
      <span class="ide-statusbar__ref">{{ statusBar.ref }}</span>
    </span>
  </footer>
</template>

<style scoped>
.ide-statusbar {
  position: fixed;
  left: 0;
  right: 0;
  bottom: 0;
  z-index: var(--vp-z-index-nav);
  display: flex;
  align-items: center;
  gap: 4px;
  height: 26px;
  padding-inline: 10px;
  background: var(--ide-statusbar-bg);
  border-top: 1px solid var(--ide-border);
  font-family: var(--aq-mono);
  font-size: 11px;
  color: var(--vp-c-text-3);
  user-select: none;
}

.ide-statusbar__seg {
  display: inline-flex;
  align-items: center;
  gap: 5px;
  padding: 0 8px;
  height: 100%;
  white-space: nowrap;
  color: inherit;
  text-decoration: none;
}

a.ide-statusbar__seg:hover {
  color: var(--vp-c-brand-1);
  background: var(--vp-c-brand-soft);
}

.ide-statusbar__ready {
  color: var(--vp-c-brand-1);
}

.ide-statusbar__dot {
  width: 7px;
  height: 7px;
  border-radius: 50%;
  background: var(--vp-c-brand-1);
}

.ide-statusbar__right {
  margin-left: auto;
  display: inline-flex;
  align-items: center;
  gap: 12px;
  white-space: nowrap;
}

.ide-statusbar__ref {
  color: var(--vp-c-text-3);
}

@media (max-width: 767px) {
  .ide-statusbar__units,
  .ide-statusbar__qt,
  .ide-statusbar__ref {
    display: none;
  }

  .ide-statusbar {
    height: 24px;
    font-size: 10.5px;
  }
}
</style>
