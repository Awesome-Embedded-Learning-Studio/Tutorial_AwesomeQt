<script setup lang="ts">
// 签名时刻：1:1 HTML/CSS 复刻实例库两个真控件并 connect 起来。
// 保真参数全部来自源码：
//   widget/toggle-switch  sizeHint{52,28} · kHandlePad 3（把手 22px）· 轨道全圆角
//     轨道色 track_on rgb(0,170,0) / track_off rgb(190,190,190)，150ms OutCubic
//   widget/status-led     NORMAL rgb(0,200,0) / OFFLINE rgb(160,160,160)，300ms 过渡
// 点击开关 → LED 亮灭，caption 实时显示这条 connect 与信号参数——
// 「信号槽」这个 Qt 最核心的概念，在教程首页就能摸到。
import { ref, computed } from 'vue'

const on = ref(false)

const signal = computed(() => `toggled(${on.value}) → setOn(${on.value})`)
const ledLabel = computed(() => (on.value ? 'LED: 点亮' : 'LED: 熄灭'))

function toggle() {
  on.value = !on.value
}
</script>

<template>
  <div class="demo">
    <div class="demo__stage">
      <div class="demo__widget">
        <button
          type="button"
          class="demo__toggle"
          :class="{ 'is-on': on }"
          role="switch"
          :aria-checked="on"
          aria-label="ToggleSwitch 演示开关"
          @click="toggle"
        >
          <span class="demo__toggle-handle" />
        </button>
        <span class="demo__widget-name">ToggleSwitch</span>
      </div>

      <svg class="demo__wire" viewBox="0 0 72 12" aria-hidden="true">
        <path d="M2 6 H54" stroke-dasharray="3 3" class="demo__wire-line" />
        <path d="M54 2 L62 6 L54 10 Z" class="demo__wire-arrow" :class="{ 'is-on': on }" />
      </svg>

      <div class="demo__widget">
        <span class="demo__led" :class="{ 'is-on': on }" role="img" :aria-label="ledLabel" />
        <span class="demo__widget-name">StatusLed</span>
      </div>
    </div>

    <p class="demo__caption" aria-live="polite">
      <code>connect(toggle, &amp;ToggleSwitch::toggled, led, &amp;StatusLed::setOn);</code>
      <span class="demo__signal">{{ signal }}</span>
    </p>

    <p class="demo__src">
      复刻自实例库 <a href="#/engineering/instances/widget/toggle-switch/">widget/toggle-switch</a>
      与 <a href="#/engineering/instances/widget/status-led/">widget/status-led</a>
      成品，参数取自源码
    </p>
  </div>
</template>

<style scoped>
.demo {
  display: flex;
  flex-direction: column;
  gap: 14px;
}

.demo__stage {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 18px;
  padding: 18px 12px 10px;
}

.demo__widget {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 8px;
}

.demo__widget-name {
  font-family: var(--aq-mono);
  font-size: 10.5px;
  letter-spacing: 0.06em;
  color: var(--vp-c-text-3);
}

/* ── ToggleSwitch 复刻：52×28 轨道（sizeHint 原值）、22px 白把手、全圆角 ── */
.demo__toggle {
  position: relative;
  width: 52px;
  height: 28px;
  padding: 0;
  border: none;
  border-radius: 14px;
  background: rgb(190, 190, 190); /* track_off 源码原值 */
  cursor: pointer;
  transition: background-color 150ms cubic-bezier(0.215, 0.61, 0.355, 0.1); /* OutCubic 150ms */
  -webkit-tap-highlight-color: transparent;
}

.demo__toggle.is-on {
  background: rgb(0, 170, 0); /* track_on 源码原值 */
}

.demo__toggle:focus-visible {
  outline: 2px solid var(--vp-c-brand-1);
  outline-offset: 2px;
}

.demo__toggle-handle {
  position: absolute;
  top: 3px;             /* kHandlePad 原值 */
  left: 3px;
  width: 22px;          /* 28 - 2*3 源码同款 */
  height: 22px;
  border-radius: 50%;
  background: #ffffff;
  box-shadow: 0 1px 3px rgba(0, 0, 0, 0.3);
  transition: left 150ms cubic-bezier(0.215, 0.61, 0.355, 0.1);
}

.demo__toggle.is-on .demo__toggle-handle {
  left: 27px;           /* 3 + 52 - 6 - 22 */
}

/* ── StatusLed 复刻：18px 圆、灰↔绿 300ms ── */
.demo__led {
  width: 18px;
  height: 18px;
  border-radius: 50%;
  background: rgb(160, 160, 160); /* OFFLINE 源码原值 */
  box-shadow: inset 0 1px 2px rgba(0, 0, 0, 0.25);
  transition: background-color 300ms cubic-bezier(0.215, 0.61, 0.355, 0.1),
    box-shadow 300ms ease;
}

.demo__led.is-on {
  background: rgb(0, 200, 0); /* NORMAL 源码原值 */
  box-shadow: 0 0 10px rgba(0, 200, 0, 0.55),
    inset 0 1px 2px rgba(255, 255, 255, 0.35);
}

/* ── 连接线：信号流向 ── */
.demo__wire {
  width: 72px;
  height: 12px;
  flex-shrink: 0;
}

.demo__wire-line {
  fill: none;
  stroke: var(--vp-c-text-3);
  stroke-width: 1.4;
}

.demo__wire-arrow {
  fill: var(--vp-c-text-3);
  transition: fill 150ms ease;
}

.demo__wire-arrow.is-on {
  fill: rgb(0, 170, 0);
}

/* ── caption：真正的 connect 代码 + 实时信号 ── */
.demo__caption {
  margin: 0;
  padding: 10px 14px;
  border: 1px solid var(--ide-border);
  border-radius: 4px;
  background: var(--vp-c-bg-soft);
  font-family: var(--aq-mono);
  font-size: 11.5px;
  line-height: 1.7;
  color: var(--vp-c-text-2);
  overflow-x: auto;
}

.demo__caption code {
  color: var(--vp-c-brand-1);
  white-space: nowrap;
}

.demo__signal {
  display: block;
  margin-top: 4px;
  color: var(--vp-c-text-3);
  white-space: nowrap;
}

.demo__src {
  margin: 0;
  font-size: 12px;
  line-height: 1.7;
  color: var(--vp-c-text-3);
}

.demo__src a {
  color: var(--vp-c-text-2);
  text-decoration: none;
  border-bottom: 1px dotted var(--vp-c-text-3);
}

.demo__src a:hover {
  color: var(--vp-c-brand-1);
  border-bottom-color: var(--vp-c-brand-1);
}

@media (prefers-reduced-motion: reduce) {
  .demo__toggle,
  .demo__toggle-handle,
  .demo__led,
  .demo__wire-arrow {
    transition: none;
  }
}
</style>
