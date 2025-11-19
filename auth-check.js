// Auth check - redirect to login if not authenticated
(function() {
    // Skip check if we're on login page
    if (window.location.pathname === '/login' || window.location.pathname === '/login.html') {
        return;
    }

    // Check authentication by making a request to a protected endpoint
    fetch('/api/check-auth', {
        method: 'GET',
        credentials: 'include'
    })
    .then(response => {
        if (!response.ok || response.status === 401) {
            // Not authenticated, redirect to login
            window.location.href = '/login';
        }
    })
    .catch(() => {
        // Error or not authenticated, redirect to login
        window.location.href = '/login';
    });
})();
