<script setup lang="ts">
import { withBase } from 'vitepress'
import { layerCards } from './home-data'
</script>

<template>
  <section class="layers ide-wrap">
    <p class="ide-eyebrow layers__head">TUTORIALS · 三层递进</p>
    <div class="layers__grid">
      <a
        v-for="c in layerCards"
        :key="c.key"
        class="layers__card"
        :class="`layers__card--${c.status}`"
        :href="withBase(c.href)"
      >
        <p class="layers__label">{{ c.layer }}</p>
        <h2 class="layers__title">{{ c.title }}</h2>
        <p class="layers__subtitle">{{ c.subtitle }}</p>
        <div
          class="ide-progress layers__progress"
          role="img"
          :aria-label="`${c.title}层进度 ${c.done}/${c.total} 篇`"
        >
          <div
            class="ide-progress__fill layers__fill"
            :class="{ 'layers__fill--wip': c.status === 'wip' }"
            :style="{ width: `${Math.round((c.done / c.total) * 100)}%` }"
          />
        </div>
        <p class="layers__count">
          <template v-if="c.status === 'done'">{{ c.done }} 篇 · 全部完成</template>
          <template v-else><span class="layers__badge">连载中</span>{{ c.done }} / {{ c.total }} 篇</template>
        </p>
      </a>
    </div>
  </section>
</template>

<style scoped>
.layers {
  padding-top: 40px;
  padding-bottom: 8px;
}

.layers__head {
  margin: 0 0 16px;
}

.layers__grid {
  display: grid;
  grid-template-columns: repeat(3, 1fr);
  gap: 16px;
}

/* QGroupBox 形态：面板底 + 顶部彩色边 */
.layers__card {
  display: block;
  padding: 20px 20px 16px;
  border: 1px solid var(--ide-border);
  border-top: 3px solid var(--vp-c-brand-1);
  border-radius: 6px;
  background: var(--ide-panel-bg);
  text-decoration: none !important;
  transition: border-color 0.18s ease, box-shadow 0.18s ease;
}

.layers__card:hover {
  border-color: var(--vp-c-brand-1);
  box-shadow: 0 6px 20px rgba(0, 0, 0, 0.08);
}

.layers__card--wip {
  border-top-color: var(--aq-c-eng-1);
}

.layers__card--wip:hover {
  border-color: var(--aq-c-eng-1);
}

.layers__label {
  margin: 0 0 6px;
  font-family: var(--aq-mono);
  font-size: 10.5px;
  font-weight: 600;
  letter-spacing: 0.14em;
  color: var(--vp-c-text-3);
}

.layers__title {
  margin: 0 0 6px;
  font-size: 22px;
  font-weight: 700;
  color: var(--vp-c-text-1);
}

.layers__card:hover .layers__title {
  color: var(--vp-c-brand-1);
}

.layers__card--wip:hover .layers__title {
  color: var(--aq-c-eng-1);
}

.layers__subtitle {
  margin: 0 0 16px;
  font-size: 13px;
  line-height: 1.7;
  color: var(--vp-c-text-2);
  min-height: 44px;
}

.layers__count {
  margin: 10px 0 0;
  font-family: var(--aq-mono);
  font-size: 11.5px;
  color: var(--vp-c-text-2);
}

.layers__badge {
  display: inline-block;
  margin-right: 8px;
  padding: 1px 8px;
  border-radius: 8px;
  background: var(--aq-c-eng-soft);
  color: var(--aq-c-eng-1);
  font-size: 10.5px;
  font-weight: 600;
}

.layers__fill--wip {
  background: var(--aq-c-eng-1);
}

@media (max-width: 959px) {
  .layers__grid {
    grid-template-columns: 1fr;
  }

  .layers__subtitle {
    min-height: 0;
  }
}
</style>
