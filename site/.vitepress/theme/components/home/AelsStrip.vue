<script setup lang="ts">
// 生态定位一行：AwesomeQt 是 AELS 的 Qt 内容旗舰，前后邻居一目了然。
import { aelsLinks } from './home-data'
</script>

<template>
  <p class="aels ide-wrap">
    <span>AwesomeQt 是 Awesome-Embedded-Learning-Studio 的 Qt 内容旗舰，生态邻居：</span>
    <template v-for="(l, i) in aelsLinks" :key="l.link">
      <a :href="l.link" target="_blank" rel="noopener">{{ l.text }}</a>
      <span v-if="i < aelsLinks.length - 1" class="aels__sep">·</span>
    </template>
  </p>
</template>

<style scoped>
.aels {
  display: flex;
  flex-wrap: wrap;
  align-items: baseline;
  justify-content: center;
  gap: 6px 10px;
  margin: 0 auto;
  padding-top: 36px;
  padding-bottom: 56px;
  font-size: 12.5px;
  color: var(--vp-c-text-3);
  text-align: center;
}

.aels a {
  color: var(--vp-c-text-2);
  text-decoration: none;
  border-bottom: 1px dotted var(--vp-c-text-3);
}

.aels a:hover {
  color: var(--vp-c-brand-1);
  border-bottom-color: var(--vp-c-brand-1);
}

.aels__sep {
  color: var(--vp-c-divider);
}
</style>
