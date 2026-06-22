<script setup lang="ts">
import { ref, onMounted, onUnmounted, computed } from 'vue'

// 首页轮播：展示项目亮点。图放 tutorial/public/carousel/（VitePress 发到 /Tutorial_AwesomeQt/carousel/）。
// 图片还没到位时，<img> @error 切到品牌渐变占位（不显示破图）。
interface Slide {
  img: string
  title: string
  subtitle: string
  link: string
  icon: string
}

const slides: Slide[] = [
  {
    img: '/Tutorial_AwesomeQt/carousel/status-led.webp',
    title: '实例库 · StatusLED',
    subtitle: '状态指示灯，4 种状态 + 闪烁动画',
    link: '/widget/status-led/',
    icon: '💡',
  },
  {
    img: '/Tutorial_AwesomeQt/carousel/toggle-switch.webp',
    title: '实例库 · 自定义开关',
    subtitle: 'toggle-switch 控件，开箱即用的 AwesomeQt:: 组件',
    link: '/widget/toggle-switch/',
    icon: '🎚️',
  },
  {
    img: '/Tutorial_AwesomeQt/carousel/tutorial.webp',
    title: '三层递进教程',
    subtitle: '入门 137 · 进阶 134 · 专家进行中',
    link: '/beginner/',
    icon: '📚',
  },
  {
    img: '/Tutorial_AwesomeQt/carousel/expert.webp',
    title: '专家层 · 源码可核对',
    subtitle: '每条结论带 文件:行号 证据，读懂 Qt 源码',
    link: '/expert/',
    icon: '🔬',
  },
]

const current = ref(0)
const paused = ref(false)
// 哪些 slide 的图加载失败了（切占位）
const failed = ref<Set<number>>(new Set())
let timer: ReturnType<typeof setInterval> | null = null

const total = computed(() => slides.length)

function go(i: number) {
  current.value = (i + total.value) % total.value
}
function next() {
  go(current.value + 1)
}
function prev() {
  go(current.value - 1)
}
function onImgError(i: number) {
  failed.value.add(i)
}

onMounted(() => {
  timer = setInterval(() => {
    if (!paused.value) next()
  }, 5000)
})
onUnmounted(() => {
  if (timer) clearInterval(timer)
})
</script>

<template>
  <div
    class="home-carousel"
    @mouseenter="paused = true"
    @mouseleave="paused = false"
  >
    <div class="home-carousel__viewport">
      <a
        v-for="(s, i) in slides"
        :key="i"
        class="home-carousel__slide"
        :class="{ active: i === current, placeholder: failed.has(i) }"
        :href="s.link"
      >
        <!-- 有图：显示图；图加载失败或还没提供：显示品牌渐变占位 -->
        <img
          v-if="!failed.has(i)"
          :src="s.img"
          :alt="s.title"
          loading="lazy"
          @error="onImgError(i)"
        />
        <div v-else class="home-carousel__ph">
          <span class="home-carousel__ph-icon">{{ s.icon }}</span>
        </div>
        <div class="home-carousel__caption">
          <div class="home-carousel__title">{{ s.title }}</div>
          <div class="home-carousel__subtitle">{{ s.subtitle }}</div>
        </div>
      </a>
    </div>

    <button class="home-carousel__arrow prev" aria-label="上一张" @click.prevent="prev">‹</button>
    <button class="home-carousel__arrow next" aria-label="下一张" @click.prevent="next">›</button>

    <div class="home-carousel__dots">
      <button
        v-for="(s, i) in slides"
        :key="i"
        class="home-carousel__dot"
        :class="{ active: i === current }"
        :aria-label="`第 ${i + 1} 张`"
        @click.prevent="go(i)"
      />
    </div>
  </div>
</template>

<style scoped>
.home-carousel {
  position: relative;
  width: 100%;
  max-width: 960px;
  margin: 0 auto 32px;
  border-radius: 14px;
  overflow: hidden;
  box-shadow: 0 4px 24px rgba(0, 0, 0, 0.1);
}

.home-carousel__viewport {
  position: relative;
  aspect-ratio: 16 / 9;
  background: var(--vp-c-bg-soft);
}

.home-carousel__slide {
  position: absolute;
  inset: 0;
  opacity: 0;
  transition: opacity 0.6s ease;
  display: flex;
  text-decoration: none;
}

.home-carousel__slide.active {
  opacity: 1;
}

.home-carousel__slide img {
  width: 100%;
  height: 100%;
  object-fit: cover;
  display: block;
}

/* 占位（图还没提供时）：品牌渐变 + 大 emoji */
.home-carousel__ph {
  width: 100%;
  height: 100%;
  display: flex;
  align-items: center;
  justify-content: center;
  background: linear-gradient(135deg, var(--vp-c-brand-soft) 0%, var(--vp-c-indigo-soft) 100%);
}

.home-carousel__ph-icon {
  font-size: 4rem;
  opacity: 0.6;
}

.home-carousel__caption {
  position: absolute;
  left: 0;
  right: 0;
  bottom: 0;
  padding: 20px 24px;
  background: linear-gradient(to top, rgba(0, 0, 0, 0.7), transparent);
  color: #fff;
}

.home-carousel__title {
  font-size: 1.15rem;
  font-weight: 700;
  margin-bottom: 4px;
}

.home-carousel__subtitle {
  font-size: 0.88rem;
  opacity: 0.9;
}

.home-carousel__arrow {
  position: absolute;
  top: 50%;
  transform: translateY(-50%);
  width: 36px;
  height: 36px;
  border: 0;
  border-radius: 50%;
  background: rgba(0, 0, 0, 0.35);
  color: #fff;
  font-size: 1.4rem;
  line-height: 1;
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  transition: background 0.2s ease;
  z-index: 2;
}

.home-carousel__arrow:hover {
  background: rgba(0, 0, 0, 0.6);
}

.home-carousel__arrow.prev {
  left: 12px;
}

.home-carousel__arrow.next {
  right: 12px;
}

.home-carousel__dots {
  position: absolute;
  bottom: 12px;
  left: 50%;
  transform: translateX(-50%);
  display: flex;
  gap: 6px;
  z-index: 2;
}

.home-carousel__dot {
  width: 8px;
  height: 8px;
  padding: 0;
  border: 0;
  border-radius: 50%;
  background: rgba(255, 255, 255, 0.5);
  cursor: pointer;
  transition: background 0.2s ease, transform 0.2s ease;
}

.home-carousel__dot.active {
  background: #fff;
  transform: scale(1.25);
}

@media (max-width: 768px) {
  .home-carousel__title {
    font-size: 1rem;
  }
  .home-carousel__subtitle {
    font-size: 0.78rem;
  }
  .home-carousel__arrow {
    width: 30px;
    height: 30px;
    font-size: 1.1rem;
  }
}
</style>
