<script lang="ts">
    import { notification } from '$lib/stores/notification';
    let username = '';
    let password = '';
  
    async function handleLogin() {
      try {
        const res = await fetch(`${import.meta.env.PUBLIC_API_BASE_URL}/api/auth/login`, {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ username, password })
        });
        if (!res.ok) {
          throw new Error('Server responded with an error');
        }
        // Process response on success
      } catch (error) {
        console.error('Connection error:', error);
        notification.set('Failed to connect to server'); // This triggers the popup
      }
    }
  </script>
  
  <form on:submit|preventDefault={handleLogin}>
    <label>
      Username:
      <input type="text" bind:value={username} required />
    </label>
    <label>
      Password:
      <input type="password" bind:value={password} required />
    </label>
    <button type="submit">Log In</button>
  </form>
  