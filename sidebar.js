// sidebar.js - loads shared sidebar fragment and marks active nav item
(function(){
    async function loadSidebar(){
        const root = document.getElementById('sidebar-root');
        if(!root) return;
        try{
            // use absolute path with cache busting to avoid stale content
            const resp = await fetch('/sidebar.html?v=' + Date.now());
            if(!resp.ok) throw new Error('Sidebar not found');
            const html = await resp.text();
            root.innerHTML = html;
            markActive();
        }catch(e){
            // fallback: inject inline sidebar markup so pages opened via file:// or different routes still show nav
            console.warn('Could not load sidebar fragment, injecting fallback sidebar:', e);
            // Also inject CSS link for fallback
            if(!document.querySelector('link[href="sidebar.css"]')){
                const link = document.createElement('link');
                link.rel = 'stylesheet';
                link.href = 'sidebar.css';
                document.head.appendChild(link);
            }
            root.innerHTML = `
                <aside class="sidebar">
                    <div class="brand">
                        <div class="logo"></div>
                        <div>
                            <h2>CORE</h2>
                            <div style="font-size:13px;opacity:0.95">Conductive Suit Reliability Evaluator</div>
                        </div>
                    </div>
                    <ul class="nav">
                        <li><a href="dashboard.html">Dashboard</a></li>
                        <li><a href="pengujian.html">Pengujian</a></li>
                        <li><a href="daftar.html">Data Konduktif</a></li>
                        <li><a href="histori.html">Histori</a></li>
                        <li><a href="foto.html">Foto Conductive Suite</a></li>
                        <li><a href="user.html">User Management</a></li>
                    </ul>
                    <div style="padding:16px;margin-top:auto;border-top:1px solid rgba(255,255,255,0.1);">
                        <button onclick="logout()" style="width:100%;background:#ef4444;color:white;padding:10px;border-radius:8px;border:none;font-weight:600;cursor:pointer;transition:all 0.3s;font-size:14px;">
                            🚪 Logout
                        </button>
                    </div>
                </aside>
            `;
            markActive();
        }
    }

    function markActive(){
        try{
            const path = (window.location.pathname.split('/').pop() || 'index.html').toLowerCase();
            document.querySelectorAll('.nav li').forEach(li=>{
                li.classList.remove('active');
                const a = li.querySelector('a');
                if(!a) return;
                const href = a.getAttribute('href') || '';
                if(href.split('/').pop().toLowerCase() === path) li.classList.add('active');
            });
        }catch(e){/* ignore */}
    }

    // Run on DOMContentLoaded with retry logic
    function init(){
        if(document.readyState === 'loading'){
            document.addEventListener('DOMContentLoaded', loadSidebar);
        } else {
            // DOM already loaded, but check if sidebar-root exists
            if(document.getElementById('sidebar-root')){
                loadSidebar();
            } else {
                // retry after a short delay in case the element isn't rendered yet
                setTimeout(loadSidebar, 50);
            }
        }
    }
    
    init();
})();
