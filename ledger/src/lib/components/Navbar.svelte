<script lang="ts">
  import { browser } from '$app/environment';
  import { page } from '$app/stores';
  import { onMount } from 'svelte';
  import { PUBLIC_API_BASE_URL } from '$env/static/public';
// Tells us if we're in the browser
  import { goto } from '$app/navigation';
  import {
    Button,
    Collapse,
    Dropdown,
    DropdownItem,
    DropdownMenu,
    DropdownToggle,
    Input,
    Nav,
    Navbar,
    NavbarBrand,
    NavbarToggler,
    NavItem,
    NavLink
  } from '@sveltestrap/sveltestrap';

  let isOpen = false;
  let isLoggedIn = false;
  let username = "";

  // Step 1: use a function to update login state from sessionStorage
  function checkSessionStorage() {
    if (!browser) return; // Only run in the browser
    const token = sessionStorage.getItem("userToken");
    if (token) {
      isLoggedIn = true;
      username = sessionStorage.getItem("username") || "";
    } else {
      isLoggedIn = false;
      username = "";
    }
  }

// Automatically update searchResults as the user types
$: if (searchQuery.trim()) {
  searchResults = allClasses.filter(cls =>
    cls.title.toLowerCase().includes(searchQuery.toLowerCase())
  );
} else {
  searchResults = [];
}

  // Step 2: onMount ensures it runs only in the browser after SSR
  onMount(() => {
    checkSessionStorage();
  });

  // Step 3: Reactively re-check when the URL changes (i.e. new route)
  $: if ($page.url) {
    checkSessionStorage();
  }

  function handleUpdate(event: CustomEvent<boolean>) {
    isOpen = event.detail;
  }

  function signOut() {
    if (browser) {
      sessionStorage.removeItem("userToken");
      sessionStorage.removeItem("username");
    }
    isLoggedIn = false;
    goto("/log-in"); // Redirect to login page after sign-out
  }

// --- Additional Variables for Dynamic Class Search ---
let searchQuery = "";
let allClasses: Array<{ id: string; title: string }> = [];
let searchResults: Array<{ id: string; title: string }> = [];

// --- Fetch classes from the API (if not already fetching elsewhere) ---
async function fetchClasses() {
  try {
    const res = await fetch(`${PUBLIC_API_BASE_URL}/api/me/classes`);
    if (!res.ok) {
      throw new Error(`Error: ${res.status}`);
    }
    const data = await res.json();
    allClasses = Object.values(data.classes);
    console.log("Fetched classes:", allClasses);
  } catch (err: any) {
    console.error(err);
  }
}

// Ensure classes are fetched on mount
onMount(() => {
  fetchClasses();
});

// --- Function to Select a Class (navigates to class page) ---
function selectClass(cls: { id: string; title: string }) {
  goto(`/classes/${cls.id}`);
  searchQuery = "";
}

</script>

<Navbar color="secondary-subtle" light expand="md" container="md">
  <NavbarBrand href="/">Folium</NavbarBrand>

<!-- Replace your search Input wrapper with this snippet -->
<div class="search-container" style="position: relative;">
  <Input type="text" placeholder="Search" class="mr-sm-2" bind:value={searchQuery} />
  {#if searchResults.length > 0}
    <div class="search-dropdown">
      {#each searchResults as cls}
        <div class="dropdown-item" on:click={() => selectClass(cls)}>
          {cls.title}
        </div>
      {/each}
    </div>
  {/if}
</div>
  <!-- Navbar Toggler for Mobile Views -->
  <NavbarToggler on:click={() => (isOpen = !isOpen)} />

  <!-- Collapsible Navbar -->
  <Collapse {isOpen} navbar expand="md" on:update={handleUpdate}>
    <Nav class="ms-auto" navbar>
      {#if isLoggedIn}
        <!-- Logged in: Show Profile + Sign Out -->
        <NavItem>
          <NavLink href="/profile">
            <Button color="primary">Profile</Button>
          </NavLink>
        </NavItem>
        <NavItem>
          <NavLink on:click={signOut} style="cursor: pointer;">
            <Button color="secondary">Sign Out</Button>
          </NavLink>
        </NavItem>
      {:else}
        <!-- Not logged in: Show Create Account + Log In -->
        <NavItem>
          <NavLink href="/register">
            <Button color="primary">Create Account</Button>
          </NavLink>
        </NavItem>
        <NavItem>
          <NavLink href="/log-in">
            <Button color="secondary">Log In</Button>
          </NavLink>
        </NavItem>
      {/if}
      <!-- Dropdown for More Options -->
      <Dropdown nav inNavbar>
        <DropdownToggle nav caret>More</DropdownToggle>
        <DropdownMenu end>
          <DropdownItem href="/help">Help</DropdownItem>
          <DropdownItem href="/about">About</DropdownItem>
          <DropdownItem href="/contact">Contact</DropdownItem>
        </DropdownMenu>
      </Dropdown>
    </Nav>
  </Collapse>
</Navbar>

<style>
  .ms-auto {
    margin-left: auto;
  }

  /* New styles for the search box with dynamic dropdown */
.search-container {
  width: 300px;
  position: relative;
}

.search-dropdown {
  position: absolute;
  top: 100%;
  left: 0;
  right: 0;
  background: white;
  border: 1px solid #ddd;
  border-top: none;
  max-height: 200px;
  overflow-y: auto;
  z-index: 1050;
}

.search-dropdown .dropdown-item {
  padding: 0.5rem 1rem;
  cursor: pointer;
  border-bottom: 1px solid #eee;
}

.search-dropdown .dropdown-item:hover {
  background-color: #f8f9fa;
}

</style>
