// Login Page Script
(function() {
    const loginForm = document.getElementById('loginForm');
    const usernameInput = document.getElementById('username');
    const passwordInput = document.getElementById('password');
    const btnText = document.getElementById('btnText');
    const alertBox = document.getElementById('alertBox');

    function showAlert(message, type = 'error') {
        alertBox.textContent = message;
        alertBox.className = type === 'error' 
            ? 'block p-3 rounded-lg mb-5 text-sm bg-red-50 text-red-600 border border-red-200'
            : 'block p-3 rounded-lg mb-5 text-sm bg-green-50 text-green-600 border border-green-200';
        
        if (type === 'success') {
            setTimeout(() => alertBox.className = 'hidden', 2000);
        }
    }

    loginForm.addEventListener('submit', async function(e) {
        e.preventDefault();
        
        const username = usernameInput.value.trim();
        const password = passwordInput.value;

        if (!username || !password) {
            showAlert('❌ Username dan password harus diisi!', 'error');
            return;
        }

        const submitBtn = loginForm.querySelector('button[type="submit"]');
        submitBtn.disabled = true;
        submitBtn.style.opacity = '0.6';
        btnText.textContent = '⏳ Memproses...';

        try {
            const response = await fetch('/api/login', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ username, password })
            });

            const data = await response.json();

            if (response.ok && data.success) {
                showAlert('✅ Login berhasil! Redirect ke dashboard...', 'success');
                setTimeout(() => window.location.href = '/dashboard', 1000);
            } else {
                showAlert('❌ ' + (data.message || 'Username atau password salah!'), 'error');
                submitBtn.disabled = false;
                submitBtn.style.opacity = '1';
                btnText.textContent = 'Login';
                passwordInput.value = '';
                passwordInput.focus();
            }
        } catch (error) {
            console.error('Login error:', error);
            showAlert('❌ Terjadi kesalahan. Silakan coba lagi.', 'error');
            submitBtn.disabled = false;
            submitBtn.style.opacity = '1';
            btnText.textContent = 'Login';
        }
    });

    usernameInput.addEventListener('input', () => alertBox.className = 'hidden');
    passwordInput.addEventListener('input', () => alertBox.className = 'hidden');
})();
