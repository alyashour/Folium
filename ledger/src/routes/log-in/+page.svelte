<!-- ledger/src/routes/log-in/+page.svelte -->
<script lang="ts">
  import { getContext } from 'svelte';
  
  // Get the showConnectionError function from context
  const showConnectionError = getContext('showConnectionError') as () => void;

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
      // Handle successful login...
    } catch (error) {
      console.error('Connection error:', error);
      showConnectionError(); // Trigger the error modal
    }
  }
</script>

<form on:submit|preventDefault={handleLogin}>
  <label>
    Username:
    <input type="text" bind:value={username} required />
  </label>
  <br />
  <label>
    Password:
    <input type="password" bind:value={password} required />
  </label>
  <br />
  <button type="submit">Log In</button>
</form>
