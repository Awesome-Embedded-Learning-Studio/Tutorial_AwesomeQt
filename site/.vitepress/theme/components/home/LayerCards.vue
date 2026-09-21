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
  padding-top: 50px;
  padding-bottom: 16px;
}

.layers__head {
  margin: 0 0 18px;
}

.layers__grid {
  display: grid;
  grid-template-columns: repeat(3, 1fr);
  gap: 0;
  border-top: 1px solid var(--vp-c-border);
  border-bottom: 1px solid var(--vp-c-divider);
}

/* 教程层级像一张目录表：共享边界，避免三张浮起的卡片。 */
.layers__card {
  display: block;
  padding: 26px 28px 22px;
  border-right: 1px solid var(--vp-c-divider);
  background: transparent;
  text-decoration: none !important;
  transition: background-color 0.18s ease;
}

.layers__card:last-child {
  border-right: 0;
}

.layers__card:hover {
  background: var(--vp-c-bg-soft-up);
}

.layers__card--wip {
  background: color-mix(in srgb, var(--aq-c-eng-soft) 45%, transparent);
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
  margin: 0 0 8px;
  font-size: 24px;
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
  margin: 0 0 22px;
  font-size: 14px;
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
  border-radius: 2px;
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

  .layers__card,
  .layers__card:last-child {
    border-right: 0;
    border-bottom: 1px solid var(--vp-c-divider);
  }

  .layers__card:last-child {
    border-bottom: 0;
  }

  .layers__subtitle {
    min-height: 0;
  }
}
</style>
