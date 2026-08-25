<script setup lang="ts">
// 编译输出面板：Creator 底部输出面板形态（不是假终端窗口，随主题明暗）。
// 内容为真实 CI 会话快照（见 home-data 溯源注释）。桌面默认展开，窄屏默认收起。
import { ref, onMounted } from 'vue'
import { ideSession } from './home-data'

const open = ref(true)

onMounted(() => {
  if (window.matchMedia('(max-width: 767px)').matches) {
    open.value = false
  }
})
</script>

<template>
  <section class="ide-panel output ide-wrap">
    <div class="output__bar">
      <button
        type="button"
        class="output__chevron"
        :aria-expanded="open"
        aria-label="展开或收起编译输出"
        @click="open = !open"
      >
        <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.6" stroke-linecap="round" stroke-linejoin="round"><polyline points="9 18 15 12 9 6" /></svg>
      </button>

      <span class="output__tab is-active">编译输出</span>
      <span class="output__tab">问题 (0)</span>
      <span class="output__bar-right">build-examples · ubuntu-latest</span>
    </div>

    <div v-show="open" class="output__body">
      <div v-for="(l, i) in ideSession" :key="i" class="output__line" :class="`output__line--${l.kind}`">
        <template v-if="l.kind === 'cmd'"><span class="output__ps">$&nbsp;</span>{{ l.text }}</template>
        <template v-else>{{ l.text }}</template>
      </div>
    </div>
  </section>
</template>

<style scoped>
.output {
  margin-top: 48px;
}

.output__bar {
  display: flex;
  align-items: center;
  gap: 4px;
  padding: 6px 10px;
  border-bottom: 1px solid var(--ide-border);
  background: var(--vp-c-bg-soft);
}

.output__chevron {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  width: 22px;
  height: 22px;
  border: none;
  border-radius: 4px;
  background: transparent;
  color: var(--vp-c-text-3);
  cursor: pointer;
  transition: transform 0.18s ease, color 0.18s ease, background-color 0.18s ease;
}

.output__chevron:hover {
  color: var(--vp-c-text-1);
  background: var(--vp-c-bg-mute);
}

.output__chevron[aria-expanded='true'] {
  transform: rotate(90deg);
}

.output__tab {
  padding: 3px 12px;
  font-family: var(--aq-mono);
  font-size: 11px;
  letter-spacing: 0.04em;
  color: var(--vp-c-text-3);
  white-space: nowrap;
}

.output__tab.is-active {
  color: var(--vp-c-text-1);
  border-bottom: 2px solid var(--ide-tab-active);
  font-weight: 600;
}

.output__bar-right {
  margin-left: auto;
  font-family: var(--aq-mono);
  font-size: 10.5px;
  color: var(--vp-c-text-3);
  white-space: nowrap;
}

.output__body {
  padding: 12px 16px 14px;
  font-family: var(--aq-mono);
  font-size: 12px;
  line-height: 1.9;
  overflow-x: auto;
}

.output__line {
  color: var(--vp-c-text-1);
  white-space: pre-wrap;
  overflow-wrap: anywhere;
}

.output__ps {
  color: var(--vp-c-brand-1);
  user-select: none;
}

.output__line--out {
  color: var(--vp-c-brand-1);
}

.output__line--dim {
  color: var(--vp-c-text-3);
}

.output__line--stamp {
  display: inline-block;
  margin-top: 4px;
  padding: 1px 9px;
  color: var(--vp-c-brand-1);
  border: 1px solid var(--vp-c-brand-1);
  border-radius: 3px;
  font-size: 11px;
}
</style>
