<script lang="ts">
    import { onMount } from 'svelte';
    import { Button } from '@sveltestrap/sveltestrap';
    import { PUBLIC_API_BASE_URL } from '$env/static/public';
    
    let classes: Array<{ id: string; title: string }> = [];
    let loading: boolean = true;
    let error: string | null = null;
    
    async function fetchClasses() {
      loading = true;
      error = null;
      try {
        const res = await fetch(`${PUBLIC_API_BASE_URL}/api/me/classes`);
        if (!res.ok) {
          throw new Error(`Error: ${res.status}`);
        }
        const data = await res.json();
        // Convert the object of classes into an array:
        classes = Object.values(data.classes);
      } catch (err: any) {
        error = err.message;
      } finally {
        loading = false;
      }
    }
    
    onMount(() => {
      fetchClasses();
    });
  </script>
  
  <h1>Your Classes</h1>
  
  {#if loading}
    <p>Loading...</p>
  {:else if error}
    <p>Error: {error}</p>
  {:else if classes.length === 0}
    <p>No classes found.</p>
  {:else}
    <div class="tile-container">
      {#each classes as c}
        <a href={`/classes/${c.id}`} class="class-tile">
          <h3>{c.title || "Untitled"}</h3>
        </a>
      {/each}
    </div>
  {/if}
  
  <Button class="mt-5">Add New Class</Button>
  
  <style>
    /* Flex container for the tiles */
    .tile-container {
      display: flex;
      flex-wrap: wrap;
      gap: 1rem;
    }
    
    /* Styling each tile */
    .class-tile {
      display: block;
      text-decoration: none;
      color: inherit;
      border: 1px solid #ddd;
      border-radius: 8px;
      padding: 1rem;
      background-color: #f8f9fa;
      flex: 1 1 200px; /* Each tile will have a minimum width of 200px */
      transition: transform 0.2s, background-color 0.2s;
    }
    
    .class-tile:hover {
      background-color: #e2e6ea;
      transform: translateY(-3px);
    }
    
    .class-tile h3 {
      margin: 0;
      font-size: 1.2rem;
    }
  </style>
  