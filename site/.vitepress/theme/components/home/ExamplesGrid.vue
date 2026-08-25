<script setup lang="ts">
// 示例画廊：Creator Examples 浏览器形态。chips 可过滤（全部/widget/app/model），
// industrial 栏成品文档未建，chip 呈禁用态并如实标注「预告」——不冒充有货。
import { ref, computed } from 'vue'
import { withBase } from 'vitepress'
import { examplesTiles } from './home-data'

type Lib = 'all' | 'widget' | 'app' | 'model'
const lib = ref<Lib>('all')

const chips: { key: Lib; label: string; disabled?: boolean; title?: string }[] = [
  { key: 'all', label: `全部 ${examplesTiles.length}` },
  { key: 'widget', label: 'widget 13' },
  { key: 'app', label: 'app 7' },
  { key: 'model', label: 'model 5' },
  { key: 'industrial' as Lib, label: 'industrial · 预告', disabled: true, title: '成品文档建设中' },
]

const shown = computed(() =>
  lib.value === 'all' ? examplesTiles : examplesTiles.filter((t) => t.lib === lib.value)
)
</script>

<template>
  <section class="examples ide-wrap">
    <div class="examples__head">
      <p class="ide-eyebrow">EXAMPLES · 实例库</p>
      <div class="examples__chips" role="group" aria-label="按栏过滤">
        <button
          v-for="c in chips"
          :key="c.key"
          type="button"
          class="examples__chip"
          :class="{ 'is-active': lib === c.key }"
          :aria-pressed="lib === c.key"
          :disabled="c.disabled"
          :title="c.title"
          @click="lib = c.key as Lib"
        >
          {{ c.label }}
        </button>
      </div>
    </div>

    <div class="examples__grid">
      <a
        v-for="t in shown"
        :key="`${t.lib}-${t.name}`"
        class="examples__tile"
        :href="withBase(`/engineering/instances/${t.lib}/${t.name}/`)"
      >
        <span v-if="t.img" class="examples__thumb">
          <img :src="withBase(t.img)" :alt="t.zh" loading="lazy" decoding="async" draggable="false" />
        </span>
        <span v-else class="examples__thumb examples__thumb--empty" aria-hidden="true">
          <span class="examples__thumb-glyph">{{ t.zh.slice(0, 1) }}</span>
        </span>
        <span class="examples__info">
          <span class="examples__name">{{ t.zh }}<i class="examples__lib">{{ t.lib }}</i></span>
          <span class="examples__note">{{ t.note }}</span>
        </span>
      </a>
    </div>

    <p class="examples__more">
      <a :href="withBase('/engineering/')">查看全部实例与两套文档（成品导览 / 手搓手册） →</a>
    </p>
  </section>
</template>

<style scoped>
.examples {
  padding-top: 48px;
}

.examples__head {
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 16px;
  margin-bottom: 14px;
  flex-wrap: wrap;
}

.examples__chips {
  display: flex;
  gap: 6px;
  overflow-x: auto;
  padding-bottom: 2px;
}

.examples__chip {
  flex-shrink: 0;
  padding: 3px 12px;
  border: 1px solid var(--ide-border);
  border-radius: 12px;
  background: var(--ide-panel-bg);
  font-family: var(--aq-mono);
  font-size: 11.5px;
  color: var(--vp-c-text-2);
  cursor: pointer;
  transition: border-color 0.15s ease, color 0.15s ease, background-color 0.15s ease;
}

.examples__chip:hover:not(:disabled) {
  border-color: var(--vp-c-brand-1);
  color: var(--vp-c-brand-1);
}

.examples__chip.is-active {
  border-color: var(--vp-c-brand-1);
  background: var(--vp-c-brand-soft);
  color: var(--vp-c-brand-1);
  font-weight: 600;
}

.examples__chip:disabled {
  opacity: 0.45;
  cursor: not-allowed;
}

.examples__grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(210px, 1fr));
  gap: 14px;
}

.examples__tile {
  display: flex;
  flex-direction: column;
  border: 1px solid var(--ide-border);
  border-radius: 6px;
  background: var(--ide-panel-bg);
  overflow: hidden;
  text-decoration: none !important;
  transition: border-color 0.18s ease, box-shadow 0.18s ease;
}

.examples__tile:hover {
  border-color: var(--vp-c-brand-1);
  box-shadow: 0 6px 20px rgba(0, 0, 0, 0.08);
}

.examples__thumb {
  display: block;
  aspect-ratio: 16 / 10;
  background: var(--vp-c-bg-soft);
}

.examples__thumb img {
  display: block;
  width: 100%;
  height: 100%;
  object-fit: cover;
}

.examples__thumb--empty {
  display: flex;
  align-items: center;
  justify-content: center;
  background: repeating-linear-gradient(
    -45deg,
    var(--vp-c-bg-soft),
    var(--vp-c-bg-soft) 8px,
    var(--vp-c-bg-mute) 8px,
    var(--vp-c-bg-mute) 9px
  );
}

.examples__thumb-glyph {
  font-size: 30px;
  font-weight: 700;
  color: var(--vp-c-text-3);
  opacity: 0.6;
}

.examples__info {
  display: flex;
  flex-direction: column;
  gap: 3px;
  padding: 10px 12px 12px;
}

.examples__name {
  display: flex;
  align-items: center;
  gap: 6px;
  font-size: 13.5px;
  font-weight: 600;
  color: var(--vp-c-text-1);
}

.examples__tile:hover .examples__name {
  color: var(--vp-c-brand-1);
}

.examples__lib {
  font-style: normal;
  padding: 0 6px;
  border: 1px solid var(--ide-border);
  border-radius: 3px;
  font-family: var(--aq-mono);
  font-size: 10px;
  color: var(--vp-c-text-3);
}

.examples__note {
  font-size: 12px;
  line-height: 1.6;
  color: var(--vp-c-text-2);
}

.examples__more {
  margin: 18px 0 0;
  font-size: 13px;
  text-align: center;
}

.examples__more a {
  color: var(--vp-c-text-2);
  text-decoration: none;
  border-bottom: 1px dotted var(--vp-c-text-3);
}

.examples__more a:hover {
  color: var(--vp-c-brand-1);
  border-bottom-color: var(--vp-c-brand-1);
}

@media (max-width: 767px) {
  .examples__grid {
    grid-template-columns: repeat(auto-fill, minmax(150px, 1fr));
  }
}
</style>
