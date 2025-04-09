<script lang="ts">
  import { onMount } from 'svelte';
  import foliumService from './FoliumService';
  import { writable } from 'svelte/store';

  const results = writable<string[]>([]);
  const error = writable<string | null>(null);

  onMount(async () => {
    try {
      results.update((prev) => [...prev, 'Testing login...']);
      const loginResult = await foliumService.login('admin', 'password');
      results.update((prev) => [...prev, `Login successful: ${JSON.stringify(loginResult)}`]);

      const token = loginResult.token;

      results.update((prev) => [...prev, 'Testing getMyClasses...']);
      const classesResult = await foliumService.getMyClasses(token);
      results.update((prev) => [...prev, `Classes: ${JSON.stringify(classesResult)}`]);

      // Add more tests as needed
    } catch (err) {
      error.set(err instanceof Error ? err.message : String(err));
    }
  });
</script>

<div>
  <h1>FoliumService Test</h1>
  {#if $error}
    <p style="color: red">Error: {$error}</p>
  {/if}
  {#each $results as result}
    <p>{result}</p>
  {/each}
</div>