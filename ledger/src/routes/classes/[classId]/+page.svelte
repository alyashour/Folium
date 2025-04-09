<script lang="ts">
    import { onMount } from "svelte";
    import { PUBLIC_API_BASE_URL } from "$env/static/public";
    import Note from "$lib/components/Note.svelte";
    import { fade } from 'svelte/transition';
    import {
      Button,
      Col,
      Row,
      Dropdown,
      DropdownToggle,
      DropdownMenu,
      DropdownItem,
    } from "@sveltestrap/sveltestrap";
    import { page } from "$app/stores";
  
    // Get the dynamic parameter (classId) from the URL.
    let classId: string;
    $: classId = $page.params.classId;
  
    // Appearance dropdown toggling functions
    let textSizeDropdownOpen = false;
    let widthDropdownOpen = false;
    let colorDropdownOpen = false;

  // Modal state and input data for "Add New Class"
  let joinModal = false;
let joinDropdownOpen = false;
let selectedClassTitle = "";
let joinMessage: string = "";

let textSize = "medium";
let colorMode: "light" | "dark" = "light";
  
    function toggleTextSize() {
      textSizeDropdownOpen = !textSizeDropdownOpen;
    }
    function toggleWidth() {
      widthDropdownOpen = !widthDropdownOpen;
    }
    function toggleColor() {
      colorDropdownOpen = !colorDropdownOpen;
    }
  
    // State for classes list (left column)
    let classes: Array<{ id: string; title: string }> = [];
    let classesLoading: boolean = true;
    let classesError: string | null = null;
  
    async function fetchClasses() {
      classesLoading = true;
      classesError = null;
      try {
        const url = `${PUBLIC_API_BASE_URL}/api/me/classes`;
        const res = await fetch(url);
        if (!res.ok) {
          throw new Error(`Error: ${res.status}`);
        }
        let data = await res.json();
        // Convert the object to an array of class objects.
        classes = Object.values(data.classes);
      } catch (err: any) {
        classesError = err.message;
      } finally {
        classesLoading = false;
      }
    }
  
    onMount(() => {
    fetchClasses();
  });

  function openJoinModal() {
  joinModal = true;

}

// Function to close the join modal and reset selection
function closeJoinModal() {
  joinModal = false;
  selectedClassTitle = "";
}

// Toggle the dropdown open/closed
function toggleJoinDropdown() {
  joinDropdownOpen = !joinDropdownOpen;
}

// Function called when a class is selected from the dropdown
function selectClass(cls: { id: string; title: string }) {
  selectedClassTitle = cls.title;
  // For now, simply navigate to the selected class’s page
  goto(`/classes/${cls.id}`);
  closeJoinModal();
}

function joinClassAction() {
  if (selectedClassTitle) {
    joinMessage = `${selectedClassTitle} joined`;
  } else {
    joinMessage = "Please select a class";
  }
  closeJoinModal();
  setTimeout(() => {
    joinMessage = "";
  }, 2000);
}

function setTextSize(size: string) {
  textSize = size;
}
// Function to change color mode
function setColorMode(mode: "light" | "dark") {
  colorMode = mode;
}
  </script>
  
  <div class="content-container {colorMode}">
  <Row class="mt-3">
    <!-- Left Column: Class Selector -->
    <Col md="2">
      <div class="header-row">
        <!-- My Classes header button -->
        <Button color="primary" size="lg" outline href="/classes" class="my-classes-header">
          My Classes
        </Button>
        <!-- Refresh button placed below with a top margin -->
        <Button color="secondary" size="sm" on:click={refreshClasses} title="Refresh" class="mt-2">
          🔄 Refresh
        </Button>
      </div>
      
  
      {#if classesLoading}
        <p>Loading...</p>
      {:else if classesError}
        <p>Error: {classesError}</p>
      {:else if classes.length === 0}
        <p>No classes found.</p>
      {:else}
        <ul class="list-unstyled">
          {#each classes as c}
            <li>
              <a href={`/classes/${c.id}`}>{c.title}</a>
            </li>
          {/each}
        </ul>
      {/if}

<!-- Join Class button triggers the modal popup -->
<Button color="primary" size="sm" on:click={openJoinModal}>Join Class</Button>

<!-- Display the join message if available -->
{#if joinMessage}
  <p class="join-message" transition:fade>{joinMessage}</p>
{/if}

{#if joinModal}
  <div class="custom-modal-overlay" on:click={closeJoinModal}>
    <div class="custom-modal" on:click|stopPropagation>
      <div class="custom-modal-header">Join a Class</div>
      <div class="custom-modal-body">
        <Dropdown isOpen={joinDropdownOpen} toggle={toggleJoinDropdown}>
          <DropdownToggle caret color="primary" size="sm">
            {selectedClassTitle || "Select a Class"}
          </DropdownToggle>
          <DropdownMenu>
            {#each classes as c}
              <DropdownItem on:click={() => selectClass(c)}>
                {c.title}
              </DropdownItem>
            {/each}
          </DropdownMenu>
        </Dropdown>
      </div>
      <div class="custom-modal-footer">
        <Button color="secondary" size="sm" on:click={closeJoinModal}>Cancel</Button>
        <Button color="primary" size="sm" on:click={joinClassAction}>Join</Button>
      </div>
    </div>
  </div>
{/if}


    </Col>
  
    <!-- Main Column: Note Component -->
    <Col md="8">
      <Note {classId} />
    </Col>
  
    <!-- Right Column: Appearance Settings -->
    <Col md="2">
      <h5>Appearance</h5>
  
      <!-- Text Size Dropdown -->
      <Dropdown isOpen={textSizeDropdownOpen} toggle={toggleTextSize}>
        <DropdownToggle caret color="secondary" size="sm">
          Text Size: {textSize.charAt(0).toUpperCase() + textSize.slice(1)}
        </DropdownToggle>
        <DropdownMenu>
          <DropdownItem on:click={() => setTextSize('small')}>Small</DropdownItem>
          <DropdownItem on:click={() => setTextSize('medium')}>Medium</DropdownItem>
          <DropdownItem on:click={() => setTextSize('large')}>Large</DropdownItem>
        </DropdownMenu>
      </Dropdown>
  
      <!-- Width Dropdown -->
      <Dropdown isOpen={widthDropdownOpen} toggle={toggleWidth} class="mt-2">
        <DropdownToggle caret color="secondary" size="sm">
          Width
        </DropdownToggle>
        <DropdownMenu>
          <DropdownItem>Wide</DropdownItem>
          <DropdownItem>Narrow</DropdownItem>
        </DropdownMenu>
      </Dropdown>
  
      <!-- Color Dropdown -->
      <Dropdown isOpen={colorDropdownOpen} toggle={toggleColor} class="mt-2">
        <DropdownToggle caret color="secondary" size="sm">
          Color
        </DropdownToggle>
        <DropdownMenu>
          <DropdownItem on:click={() => setColorMode("light")}>Automatic (Light Mode)</DropdownItem>
          <DropdownItem on:click={() => setColorMode("light")}>Light Mode</DropdownItem>
          <DropdownItem on:click={() => setColorMode("dark")}>Dark Mode</DropdownItem>
        </DropdownMenu>
      </Dropdown>
    </Col>
  </Row>
</div>
  <style>
    
    /* Align header row items */
    .header-row {
      margin-bottom: 1rem;
    }
  
    /* Remove default list styling and add spacing */
    ul.list-unstyled {
      padding-left: 0;
    }
    ul.list-unstyled li {
      margin-bottom: 0.5rem;
    }
  
    /* Style the class links to look like badges */
    ul.list-unstyled li a {
      display: block;
      padding: 0.5rem;
      border: 1px solid #ddd;
      border-radius: 4px;
      background-color: #f8f9fa;
      color: #007bff;
      text-decoration: none;
      transition: background-color 0.2s;
    }
    ul.list-unstyled li a:hover {
      background-color: #e2e6ea;
    }
.my-classes-header {
  font-weight: bold;
  text-transform: uppercase;
  padding: 0.75rem 1.5rem;
  border-radius: 50px;
}
  /* Modal overlay styling */
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
  
  /* Modal content styling */
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
  .join-message {
  margin-top: 0.5rem;
  font-size: 0.9rem;
  color: #007bff; /* Choose an appropriate color */
  
}

.content-container.small {
  font-size: 0.85rem;
}

.content-container.medium {
  font-size: 1rem;
}

.content-container.large {
  font-size: 2rem;
}

.content-container.light {
  /* Light mode: default background and text */
  background-color: #fff;
  color: #000;
}

.content-container.dark {
  /* Dark mode: black background, white text */
  background-color: #000;
  color: #fff;
}

/* Optionally, if you want to target the Note component inside the container */
.content-container.dark .note {
  background-color: #000;
  color: #fff;
}

/* If your Note component already defines its colors, you may adjust there instead. */


  </style>
  