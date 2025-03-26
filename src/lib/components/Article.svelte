<script lang="ts">
  import { onMount } from 'svelte';
  import { PUBLIC_API_BASE_URL } from '$env/static/public';
  import Unit from './Unit.svelte'; // Adjust the import path as needed

  let data: any = null;
  let error: string | null = null;
  let loading: boolean = true;

  async function fetchWithTimeout(url: string, options: RequestInit = {}, timeout = 5000): Promise<Response> {
    const controller = new AbortController();
    const timer = setTimeout(() => controller.abort(), timeout);
    
    try {
      const response = await fetch(url, { ...options, signal: controller.signal });
      clearTimeout(timer);
      return response;
    } catch (err) {
      clearTimeout(timer);
      throw err;
    }
  }

  async function fetchData() {
    try {
      const response = await fetchWithTimeout(`${PUBLIC_API_BASE_URL}/api/classes/math101/bignote`, {}, 5000);
      if (!response.ok) {
        throw new Error(`Error: ${response.status}`);
      }
      data = await response.json();
    } catch (err: any) {
      error = err.name === "AbortError" ? "Request timed out" : err.message;
    } finally {
      loading = false;
    }
  }

  onMount(() => {
    fetchData();
  });
</script>

{#if loading}
  <p>Loading...</p>
{:else if error}
  <p>Error: {error}</p>
{:else}
  <h1>{data.bigNote.title}</h1>
  {#each data.bigNote.units as unit (unit.unitId)}
    <Unit title={unit.title} content={unit.content} />
  {/each}
{/if}
