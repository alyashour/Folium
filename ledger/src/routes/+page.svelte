<script lang="ts">
  import { PUBLIC_API_BASE_URL } from "$env/static/public";
  import { Button } from "@sveltestrap/sveltestrap";
  import { onMount } from "svelte";

  let classes: Array<{ id: string; title: string }> = [];
  let loading: boolean = true;
  let error: string | null = null;

  let showModal = false;
  let className = "";
  let startDate = "";
  let endDate = "";

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

    // Functions to manage the modal popup
    const openModal = () => {
    showModal = true;
  };

  const closeModal = () => {
    showModal = false;
    // Optionally clear input fields after closing
    className = "";
    startDate = "";
    endDate = "";
  };

  const addClass = () => {
    // Here you can include validation or API calls
    console.log("New class added:", className, startDate, endDate);
    // For now, simply close the modal after the button is clicked
    closeModal();
  };
</script>

<h1>Home</h1>

<h2>Your Classes</h2>

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


<Button class="mt-5" on:click={openModal}>Add New Class</Button>

{#if showModal}
  <!-- Renamed the overlay class -->
  <div class="custom-modal-overlay" on:click={closeModal}>
    <!-- Renamed the modal class -->
    <div class="custom-modal" on:click|stopPropagation>
      <div class="custom-modal-header">Add New Class</div>
      <div class="custom-modal-body">
        <div>
          <label for="className">Class Name:</label>
          <input
            id="className"
            type="text"
            bind:value={className}
            placeholder="Enter class name"
          />
        </div>
        <div>
          <label for="startDate">Start Date:</label>
          <input
            id="startDate"
            type="date"
            bind:value={startDate}
          />
        </div>
        <div>
          <label for="endDate">End Date:</label>
          <input
            id="endDate"
            type="date"
            bind:value={endDate}
          />
        </div>
      </div>
      <div class="custom-modal-footer">
        <button on:click={closeModal}>Cancel</button>
        <button on:click={addClass}>Add Class</button>
      </div>
    </div>
  </div>
{/if}

<h2>Another Header</h2>

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
    transition:
      transform 0.2s,
      background-color 0.2s;
  }

  .class-tile:hover {
    background-color: #e2e6ea;
    transform: translateY(-3px);
  }

  .class-tile h3 {
    margin: 0;
    font-size: 1.2rem;
  }

  h2 {
    margin-top: 2rem;
  }

  .custom-modal-overlay {
    position: fixed;
    top: 0;
    left: 0;
    right: 0;
    bottom: 0;
    background: rgba(0, 0, 0, 0.5);
    display: flex;
    align-items: center;
    justify-content: center;
    z-index: 100;
  }

  .custom-modal {
    background: white;
    padding: 1.5rem;
    border-radius: 8px;
    width: 320px;
    max-width: 95%;
    box-shadow: 0 2px 12px rgba(0, 0, 0, 0.4);
  }

  .custom-modal-header {
    font-size: 1.25rem;
    font-weight: bold;
    margin-bottom: 1rem;
  }

  .custom-modal-body > div {
    margin-bottom: 0.75rem;
  }

  .custom-modal-body label {
    display: block;
    margin-bottom: 0.25rem;
  }

  .custom-modal-body input {
    width: 100%;
    padding: 0.5rem;
    box-sizing: border-box;
  }

  .custom-modal-footer {
    display: flex;
    justify-content: flex-end;
    gap: 0.5rem;
  }

  button {
    padding: 0.5rem 1rem;
    font-size: 1rem;
  }
</style>
