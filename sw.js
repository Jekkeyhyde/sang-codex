const C='sang-codex-v30';
const CDN='sang-codex-cdn';
const F=['./','index.html','manifest.webmanifest','icons/icon-192.png','icons/icon-512.png','icons/apple-touch-icon.png'];
self.addEventListener('install',e=>{e.waitUntil(caches.open(C).then(c=>c.addAll(F)));self.skipWaiting();});
// удаляем только старые версии приложения; ИИ-модель (transformers-cache) и движок (CDN) не трогаем
self.addEventListener('activate',e=>{e.waitUntil(caches.keys().then(k=>Promise.all(k.filter(x=>x.startsWith('sang-codex-v')&&x!==C).map(x=>caches.delete(x)))));self.clients.claim();});
self.addEventListener('fetch',e=>{
  const u=new URL(e.request.url);
  if(e.request.method!=='GET')return;
  if(u.hostname==='cdn.jsdelivr.net'){ // движок ИИ: кэш-первым, чтобы работал офлайн
    e.respondWith(caches.open(CDN).then(c=>c.match(e.request).then(hit=>hit||fetch(e.request).then(r=>{if(r.ok)c.put(e.request,r.clone());return r;}))));
    return;
  }
  if(u.origin!==location.origin)return;
  e.respondWith(
    fetch(e.request,{cache:'no-store'})
      .then(r=>{const cp=r.clone();caches.open(C).then(c=>c.put(e.request,cp));return r;})
      .catch(()=>caches.match(e.request))
  );
});
