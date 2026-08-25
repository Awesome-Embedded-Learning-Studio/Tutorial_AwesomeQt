<script setup lang="ts">
// 应用标题栏：整个网站隐喻为一个 Qt 桌面应用，这是它的标题条。
// 挂 layout-top 插槽（Layout 第一个元素）；高度经 --vp-layout-top-height
// 由 VitePress 原生机制全链避让（导航/侧栏/正文自动下移），壳样式在 custom.css。
// 刻意不做假窗口按钮——点了没反应的关闭键是坏体验。
import { statusBar } from '../home/home-data'
</script>

<template>
  <div class="ide-titlebar" role="banner">
    <span class="ide-titlebar__left">
      <span class="ide-titlebar__mark" aria-hidden="true" />
      <span class="ide-titlebar__title">AwesomeQt — Qt 6 中文深度教程</span>
    </span>
    <span class="ide-titlebar__kit">{{ statusBar.kit }}</span>
  </div>
</template>

<style scoped>
.ide-titlebar__left {
  display: inline-flex;
  align-items: center;
  gap: 8px;
  min-width: 0;
}

.ide-titlebar__mark {
  width: 10px;
  height: 10px;
  border-radius: 2px;
  background: var(--vp-c-brand-1);
  flex-shrink: 0;
}

.ide-titlebar__title {
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.ide-titlebar__kit {
  white-space: nowrap;
  padding: 1px 8px;
  border: 1px solid var(--ide-border);
  border-radius: 3px;
  font-size: 10.5px;
  color: var(--vp-c-text-3);
}

@media (max-width: 767px) {
  .ide-titlebar__kit {
    display: none;
  }

  .ide-titlebar__title {
    font-size: 11px;
  }
}
</style>
