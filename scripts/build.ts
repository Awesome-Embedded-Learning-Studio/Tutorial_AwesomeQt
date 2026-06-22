import { exec } from 'child_process'
import {
  cpSync, mkdirSync, rmSync, writeFileSync,
  readdirSync, readFileSync, existsSync,
  symlinkSync, statSync,
} from 'fs'
import { join, resolve, relative, basename } from 'path'
import { createHash } from 'crypto'
import { createRequire } from 'module'
const require = createRequire(import.meta.url)

// ── CLI Flags ───────────────────────────────────────────────

const FORCE_REBUILD = process.argv.includes('--force') || process.argv.includes('--clean')
const CONCURRENCY = parseInt(process.env.BUILD_CONCURRENCY || '4', 10)

// ── Configuration ───────────────────────────────────────────

interface Volume {
  name: string
  srcDir: string
  urlPrefix: string
}

const VOLUMES: Volume[] = [
  { name: 'beginner-00-env', srcDir: 'beginner/00-environment-setup', urlPrefix: '/beginner/00-environment-setup' },
  { name: 'beginner-01-base', srcDir: 'beginner/01-qtbase', urlPrefix: '/beginner/01-qtbase' },
  { name: 'beginner-02-gui', srcDir: 'beginner/02-qtgui', urlPrefix: '/beginner/02-qtgui' },
  { name: 'beginner-03-widgets', srcDir: 'beginner/03-qtwidgets', urlPrefix: '/beginner/03-qtwidgets' },
  { name: 'beginner-04-network', srcDir: 'beginner/04-qtnetwork', urlPrefix: '/beginner/04-qtnetwork' },
  { name: 'beginner-05-other', srcDir: 'beginner/05-other-modules', urlPrefix: '/beginner/05-other-modules' },
  { name: 'beginner-06-qml', srcDir: 'beginner/06-qml', urlPrefix: '/beginner/06-qml' },
  { name: 'engineering-instances', srcDir: 'engineering/instances', urlPrefix: '/engineering/instances' },
  { name: 'advanced', srcDir: 'advanced', urlPrefix: '/advanced' },
  { name: 'expert', srcDir: 'expert', urlPrefix: '/expert' },
]

const PROJECT_ROOT = resolve(import.meta.dirname, '..')
const SITE_DIR = join(PROJECT_ROOT, 'site')
const MAIN_VP = join(SITE_DIR, '.vitepress')
const BUILD_TMP = join(MAIN_VP, '.build-tmp')
const CACHE_DIR = join(MAIN_VP, '.build-cache')
const MANIFEST_PATH = join(CACHE_DIR, 'manifest.json')
const DIST_FINAL = join(MAIN_VP, 'dist')
const DOCUMENTS = join(PROJECT_ROOT, 'tutorial')

// ── Logging ─────────────────────────────────────────────────

function ts(): string {
  return new Date().toISOString().substring(11, 19)
}

function log(msg: string) { console.log(`[${ts()}] ${msg}`) }
function logStep(msg: string) {
  console.log(`\n[${ts()}] ${'═'.repeat(60)}`)
  log(`  ${msg}`)
  console.log(`[${ts()}] ${'═'.repeat(60)}`)
}

function memMB(): string {
  const m = process.memoryUsage()
  return `RSS=${(m.rss / 1024 / 1024).toFixed(0)}MB Heap=${(m.heapUsed / 1024 / 1024).toFixed(0)}/${(m.heapTotal / 1024 / 1024).toFixed(0)}MB`
}

// ── Helpers ─────────────────────────────────────────────────

function ensureClean(dir: string) {
  if (existsSync(dir)) rmSync(dir, { recursive: true })
  mkdirSync(dir, { recursive: true })
}

function symlinkDir(target: string, link: string) {
  if (existsSync(link)) rmSync(link, { recursive: true })
  symlinkSync(target, link, 'dir')
}

function countMdFiles(dir: string): number {
  let count = 0
  try {
    for (const e of readdirSync(dir, { withFileTypes: true })) {
      if (e.name.startsWith('.')) continue
      const full = join(dir, e.name)
      if (e.isDirectory()) count += countMdFiles(full)
      else if (e.name.endsWith('.md')) count++
    }
  } catch { /* ignore */ }
  return count
}

function hashDir(dir: string): string {
  const h = createHash('sha256')
  function walk(d: string) {
    try {
      const entries = readdirSync(d, { withFileTypes: true }).sort((a, b) => a.name.localeCompare(b.name))
      for (const e of entries) {
        if (e.name.startsWith('.')) continue
        const full = join(d, e.name)
        if (e.isDirectory()) { walk(full); continue }
        const s = statSync(full)
        h.update(`${relative(dir, full)}:${s.size}:${s.mtimeMs}\n`)
      }
    } catch { /* ignore */ }
  }
  walk(dir)
  return h.digest('hex').substring(0, 16)
}

// ── Manifest ────────────────────────────────────────────────

interface ManifestEntry { hash: string; timestamp: string }
type Manifest = Record<string, ManifestEntry>

function readManifest(): Manifest {
  if (FORCE_REBUILD) {
    log('  --force: discarding build cache')
    if (existsSync(CACHE_DIR)) rmSync(CACHE_DIR, { recursive: true })
    return {}
  }
  if (!existsSync(MANIFEST_PATH)) return {}
  try { return JSON.parse(readFileSync(MANIFEST_PATH, 'utf-8')) } catch { return {} }
}

function writeManifest(manifest: Manifest) {
  mkdirSync(CACHE_DIR, { recursive: true })
  writeFileSync(MANIFEST_PATH, JSON.stringify(manifest, null, 2))
}

// ── Config Generators ───────────────────────────────────────

function generateVolumeConfig(vol: Volume, absSiteDir: string, absSrcDir: string): string {
  const relSrc = relative(absSiteDir, absSrcDir)
  const relOut = relative(absSiteDir, join(BUILD_TMP, 'output', vol.name))
  const vpDir = join(absSiteDir, '.vitepress')
  const relShared = relative(vpDir, join(MAIN_VP, 'config', 'shared')).replace(/\\/g, '/')

  // 共享配置（base / markdown 插件 / head / vite / vue / 主题基础项）统一来自 config/shared.ts。
  // 这里只挂分卷专属字段（srcDir / outDir / ignoreDeadLinks）。改 markdown 插件只改 shared.ts 一处。
  return `import { defineConfig } from 'vitepress'
import { sharedBase, sharedMarkdown, sharedThemeBase } from '${relShared}'

export default defineConfig({
  srcDir: '${relSrc.replace(/\\/g, '/')}',
  outDir: '${relOut.replace(/\\/g, '/')}',
  ignoreDeadLinks: true,
  ...sharedBase,
  markdown: sharedMarkdown,
  themeConfig: sharedThemeBase,
})
`
}

function generateRootConfig(absSiteDir: string, absSrcDir: string): string {
  const relSrc = relative(absSiteDir, absSrcDir)
  const relOut = relative(absSiteDir, join(BUILD_TMP, 'output', 'root'))
  const vpDir = join(absSiteDir, '.vitepress')
  const relNav = relative(vpDir, join(MAIN_VP, 'config', 'nav')).replace(/\\/g, '/')
  const relSidebar = relative(vpDir, join(MAIN_VP, 'config', 'sidebar')).replace(/\\/g, '/')
  const relShared = relative(vpDir, join(MAIN_VP, 'config', 'shared')).replace(/\\/g, '/')

  return `import { defineConfig } from 'vitepress'
import { navZh } from '${relNav}'
import { buildSidebar } from '${relSidebar}'
import { sharedBase, sharedMarkdown, sharedThemeBase } from '${relShared}'

export default defineConfig({
  srcDir: '${relSrc.replace(/\\/g, '/')}',
  outDir: '${relOut.replace(/\\/g, '/')}',
  ignoreDeadLinks: true,
  ...sharedBase,
  markdown: sharedMarkdown,
  themeConfig: { ...sharedThemeBase, nav: navZh, sidebar: buildSidebar() },
})
`
}

// ── Build Tasks ─────────────────────────────────────────────

interface BuildTask {
  id: string
  vol: Volume
  cacheKey: string
  cached: boolean
}

function prepareTask(vol: Volume, manifest: Manifest): BuildTask {
  const volDocDir = join(DOCUMENTS, vol.srcDir)
  const cacheKey = existsSync(volDocDir) ? hashDir(volDocDir) : ''
  const prev = manifest[vol.name]
  const cached = !FORCE_REBUILD && prev && prev.hash === cacheKey && existsSync(join(CACHE_DIR, 'output', vol.name))
  return { id: vol.name, vol, cacheKey, cached }
}

function execAsync(cmd: string, opts?: { cwd?: string }): Promise<void> {
  return new Promise((resolve, reject) => {
    exec(cmd, { cwd: opts?.cwd ?? PROJECT_ROOT }, (err, stdout, stderr) => {
      if (stdout) process.stdout.write(stdout)
      if (stderr) process.stderr.write(stderr)
      if (err) reject(err)
      else resolve()
    })
  })
}

async function buildVolume(task: BuildTask): Promise<string> {
  const { id, vol } = task
  const volDocDir = join(DOCUMENTS, vol.srcDir)
  const volSrcDir = join(BUILD_TMP, `src-${id}`)
  const tmpSite = join(BUILD_TMP, `site-${id}`)
  const volOutput = join(BUILD_TMP, 'output', id)
  const cachedOutput = join(CACHE_DIR, 'output', id)

  if (task.cached) {
    log(`  ${id}: ✓ cached (unchanged)`)
    mkdirSync(volOutput, { recursive: true })
    cpSync(cachedOutput, volOutput, { recursive: true })
    return volOutput
  }

  const mdCount = countMdFiles(volDocDir)
  log(`  ${id}: building ${mdCount} files...`)

  // Preserve directory structure so VitePress outputs to the correct URL path
  // e.g. beginner/03-qtwidgets/*.md stays under beginner/03-qtwidgets/ in output
  mkdirSync(join(volSrcDir, ...vol.srcDir.split('/')), { recursive: true })
  if (existsSync(volDocDir)) {
    cpSync(volDocDir, join(volSrcDir, ...vol.srcDir.split('/')), { recursive: true })
  }

  mkdirSync(join(tmpSite, '.vitepress'), { recursive: true })
  writeFileSync(join(tmpSite, '.vitepress', 'config.ts'), generateVolumeConfig(vol, tmpSite, volSrcDir))
  symlinkDir(join(MAIN_VP, 'theme'), join(tmpSite, '.vitepress', 'theme'))
  symlinkDir(join(MAIN_VP, 'plugins'), join(tmpSite, '.vitepress', 'plugins'))
  symlinkDir(join(MAIN_VP, 'public'), join(tmpSite, '.vitepress', 'public'))

  const t0 = Date.now()
  await execAsync(`pnpm exec vitepress build ${relative(PROJECT_ROOT, tmpSite)}`)
  const elapsed = ((Date.now() - t0) / 1000).toFixed(1)

  if (!existsSync(volOutput)) throw new Error(`${id}: output dir not found after build`)
  log(`  ${id}: ✓ built in ${elapsed}s (${mdCount} files, ${memMB()})`)

  mkdirSync(join(CACHE_DIR, 'output'), { recursive: true })
  if (existsSync(cachedOutput)) rmSync(cachedOutput, { recursive: true })
  cpSync(volOutput, cachedOutput, { recursive: true })

  return volOutput
}

async function runParallel<T>(tasks: T[], fn: (t: T) => Promise<void>, limit: number): Promise<void> {
  let idx = 0
  const workers: Promise<void>[] = []
  for (let i = 0; i < Math.min(limit, tasks.length); i++) {
    workers.push((async () => {
      while (idx < tasks.length) {
        const task = tasks[idx++]
        if (task) await fn(task)
      }
    })())
  }
  await Promise.all(workers)
}

// ── Cross-Volume Data Unification ────────────────────────────

function unifyCrossVolumeData(distDir: string) {
  logStep('Step 3.5/4: Unifying cross-volume hash maps & site data')

  const htmlFiles: string[] = []
  function walk(d: string) {
    for (const e of readdirSync(d, { withFileTypes: true })) {
      const full = join(d, e.name)
      if (e.isDirectory()) walk(full)
      else if (e.name.endsWith('.html')) htmlFiles.push(full)
    }
  }
  walk(distDir)
  log(`  Found ${htmlFiles.length} HTML files`)

  const mergedHashMap: Record<string, string> = {}
  let rootSiteDataExpr = ''

  for (const f of htmlFiles) {
    const c = readFileSync(f, 'utf-8')

    const hmMatch = c.match(/__VP_HASH_MAP__\s*=\s*JSON\.parse\("(.+?)"\)/)
    if (hmMatch) {
      try {
        const mapObj: Record<string, string> = JSON.parse(new Function(`return "${hmMatch[1]}"`)())
        Object.assign(mergedHashMap, mapObj)
      } catch { /* skip */ }
    }

    if (f === join(distDir, 'index.html')) {
      const sdMatch = c.match(/__VP_SITE_DATA__\s*=\s*JSON\.parse\("(.+?)"\)/)
      if (sdMatch) rootSiteDataExpr = sdMatch[1]
    }
  }

  log(`  Merged hash map: ${Object.keys(mergedHashMap).length} entries`)
  log(`  Root site data: ${rootSiteDataExpr ? 'found' : 'MISSING'}`)

  const hmJsLiteral = JSON.stringify(JSON.stringify(mergedHashMap))

  let patched = 0
  for (const f of htmlFiles) {
    let c = readFileSync(f, 'utf-8')
    let changed = false

    const hmReplace = c.replace(
      /__VP_HASH_MAP__\s*=\s*JSON\.parse\(".+?"\)/,
      `__VP_HASH_MAP__=JSON.parse(${hmJsLiteral})`
    )
    if (hmReplace !== c) { c = hmReplace; changed = true }

    if (rootSiteDataExpr && f !== join(distDir, 'index.html')) {
      const sdReplace = c.replace(
        /__VP_SITE_DATA__\s*=\s*JSON\.parse\(".+?"\)/,
        `__VP_SITE_DATA__=JSON.parse("${rootSiteDataExpr}")`
      )
      if (sdReplace !== c) { c = sdReplace; changed = true }
    }

    if (changed) {
      writeFileSync(f, c)
      patched++
    }
  }
  log(`  Patched ${patched} files with unified data`)
}

// ── Search Index Merge ──────────────────────────────────────

function extractSearchDocs(indexPath: string): Array<Record<string, unknown>> {
  const content = readFileSync(indexPath, 'utf-8')
  const m = content.match(/^const \w+=(.+);export\{/)
  if (!m) { log(`  ⚠ Could not parse: ${relative(PROJECT_ROOT, indexPath)}`); return [] }
  const jsonStr: string = new Function(`return ${m[1]}`)()
  const data = JSON.parse(jsonStr)
  const docs: Array<Record<string, unknown>> = []
  for (const [idStr, url] of Object.entries<string>(data.documentIds)) {
    const fields = data.storedFields[idStr]
    if (!fields) continue
    docs.push({ id: url, title: fields.title || '', titles: fields.titles || [] })
  }
  return docs
}

async function buildSearchIndexJs(docs: Array<Record<string, unknown>>): Promise<string> {
  const MiniSearch = require('minisearch')
  const ms = new MiniSearch({ fields: ['title', 'titles', 'text'], storeFields: ['title', 'titles'] })
  ms.addAll(docs)
  const json = JSON.stringify(ms.toJSON())
  return `const e=${JSON.stringify(json)};export{e as default};`
}

function findAllSearchIndexFiles(dir: string): string[] {
  const result: string[] = []
  const chunksDir = join(dir, 'assets', 'chunks')
  if (!existsSync(chunksDir)) return result
  for (const f of readdirSync(chunksDir)) {
    if (/^@localSearchIndexroot\.[^.]+\.js$/.test(f)) {
      result.push(join(chunksDir, f))
    }
  }
  return result
}

async function mergeSearchIndexes(outputDirs: string[], finalDist: string) {
  logStep('Step 3/4: Merging search indexes')

  const allDocs: Array<Record<string, string>> = []
  for (const dir of outputDirs) {
    const chunksDir = join(dir, 'assets', 'chunks')
    if (!existsSync(chunksDir)) continue
    for (const f of readdirSync(chunksDir)) {
      if (!/^@localSearchIndexroot\.[^.]+\.js$/.test(f)) continue
      const docs = extractSearchDocs(join(chunksDir, f))
      log(`  ${relative(PROJECT_ROOT, dir)}: ${docs.length} docs`)
      allDocs.push(...docs)
    }
  }

  if (allDocs.length === 0) { log('  No search docs found, skipping'); return }

  const seen = new Set<string>()
  const uniqueDocs = allDocs.filter(d => {
    const id = d.id as string
    if (seen.has(id)) return false
    seen.add(id)
    return true
  })
  log(`  Merging ${uniqueDocs.length} unique docs (dropped ${allDocs.length - uniqueDocs.length} duplicates)...`)

  const js = await buildSearchIndexJs(uniqueDocs)
  const allTargets = findAllSearchIndexFiles(finalDist)
  if (allTargets.length === 0) { log('  ⚠ No target index files in final dist!'); return }

  writeFileSync(allTargets[0], js)
  const canonicalName = basename(allTargets[0])
  const stub = `export{default}from"./${canonicalName}";`
  for (let i = 1; i < allTargets.length; i++) {
    writeFileSync(allTargets[i], stub)
  }
  const savedMB = ((js.length - stub.length) * (allTargets.length - 1) / 1024 / 1024).toFixed(1)
  log(`  ✓ 1 canonical + ${allTargets.length - 1} stubs (saved ${savedMB} MB)`)
}

// ── Main ────────────────────────────────────────────────────

async function main() {
  logStep('Split Build — VitePress per-volume build')
  log(`  Project:     ${PROJECT_ROOT}`)
  log(`  Concurrency: ${CONCURRENCY}`)
  log(`  Force:       ${FORCE_REBUILD}`)
  log(`  Memory:      ${memMB()}`)
  const start = Date.now()

  ensureClean(BUILD_TMP)
  ensureClean(DIST_FINAL)
  mkdirSync(join(BUILD_TMP, 'output'), { recursive: true })

  const manifest = readManifest()

  // ── Step 1: Build root ──────────────────────────────────
  logStep('Step 1/4: Building root site (index pages)')

  const rootSrcDir = join(BUILD_TMP, 'root-src')
  mkdirSync(rootSrcDir, { recursive: true })
  // Copy root-level index and layer-level index files
  for (const f of ['index.md']) {
    const s = join(DOCUMENTS, f)
    if (existsSync(s)) cpSync(s, join(rootSrcDir, f))
  }
  // Copy layer index pages
  for (const layer of ['beginner', 'engineering', 'advanced', 'expert']) {
    const s = join(DOCUMENTS, `${layer}/index.md`)
    if (existsSync(s)) {
      mkdirSync(join(rootSrcDir, layer), { recursive: true })
      cpSync(s, join(rootSrcDir, layer, 'index.md'))
    }
  }

  const rootTmpSite = join(BUILD_TMP, 'site-root')
  mkdirSync(join(rootTmpSite, '.vitepress'), { recursive: true })
  writeFileSync(join(rootTmpSite, '.vitepress', 'config.ts'), generateRootConfig(rootTmpSite, rootSrcDir))
  symlinkDir(join(MAIN_VP, 'theme'), join(rootTmpSite, '.vitepress', 'theme'))
  symlinkDir(join(MAIN_VP, 'plugins'), join(rootTmpSite, '.vitepress', 'plugins'))
  symlinkDir(join(MAIN_VP, 'public'), join(rootTmpSite, '.vitepress', 'public'))

  const rootT0 = Date.now()
  await execAsync('pnpm exec vitepress build .', { cwd: rootTmpSite })
  const rootOutput = join(BUILD_TMP, 'output', 'root')
  if (existsSync(rootOutput)) cpSync(rootOutput, DIST_FINAL, { recursive: true })
  log(`  Root: ${((Date.now() - rootT0) / 1000).toFixed(1)}s`)

  // ── Step 2: Build volumes in parallel ────────────────────
  logStep('Step 2/4: Building volumes (parallel)')

  const tasks: BuildTask[] = []
  for (const vol of VOLUMES) {
    const volDocDir = join(DOCUMENTS, vol.srcDir)
    if (!existsSync(volDocDir)) continue
    if (countMdFiles(volDocDir) === 0) continue
    tasks.push(prepareTask(vol, manifest))
  }

  const cachedCount = tasks.filter(t => t.cached).length
  const buildCount = tasks.length - cachedCount
  log(`  Tasks: ${tasks.length} total, ${cachedCount} cached, ${buildCount} to build`)
  log(`  Concurrency: ${CONCURRENCY}\n`)

  const outputDirs: string[] = [rootOutput]
  const newManifest: Manifest = {}

  await runParallel(tasks, async (task) => {
    const volOutput = await buildVolume(task)
    outputDirs.push(volOutput)
    cpSync(volOutput, DIST_FINAL, { recursive: true })
    newManifest[task.id] = { hash: task.cacheKey, timestamp: new Date().toISOString() }
  }, CONCURRENCY)

  // ── Step 3: Merge search indexes ────────────────────────
  await mergeSearchIndexes(outputDirs, DIST_FINAL)

  // ── Step 3.5: Unify hash maps and site data ─────────────
  unifyCrossVolumeData(DIST_FINAL)

  // ── Step 4: Finalize ────────────────────────────────────
  logStep('Step 4/4: Finalizing')
  rmSync(BUILD_TMP, { recursive: true })
  writeManifest(newManifest)

  let outputFiles = 0
  function countFiles(d: string) { for (const e of readdirSync(d, { withFileTypes: true })) { if (e.isDirectory()) countFiles(join(d, e.name)); else outputFiles++ } }
  countFiles(DIST_FINAL)

  const elapsed = ((Date.now() - start) / 1000).toFixed(1)
  log(`\n  ═══ Build Summary ═══`)
  log(`  Status:   ✓ SUCCESS`)
  log(`  Time:     ${elapsed}s (${cachedCount} cached, ${buildCount} built)`)
  log(`  Output:   ${relative(PROJECT_ROOT, DIST_FINAL)} (${outputFiles} files)`)
  log(`  Memory:   ${memMB()}`)
  log(`  Tip:      Use --force for full rebuild, BUILD_CONCURRENCY=N to adjust parallelism`)
}

main().catch((err) => {
  log('\n  BUILD FAILED')
  console.error(err)
  process.exit(1)
})
