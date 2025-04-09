<script lang="ts">
  import { browser } from '$app/environment';
  import { page } from '$app/stores';
  import { onMount } from 'svelte';
  import { PUBLIC_API_BASE_URL } from '$env/static/public';
  import { goto } from '$app/navigation';
  import foliumService from '../FoliumService'
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

  // Navbar state
  let isOpen = false;
  let isLoggedIn = false;
  let username = "";
  let token = "";
  
  // Search functionality
  let searchQuery = "";
  let searchResults: Array<{ id: string; title: string }> = [];
  let allClasses: Array<{ id: string; title: string }> = [];
  
  // Check authentication status from session storage
  function checkAuthStatus() {
    if (!browser) return;
    
    token = sessionStorage.getItem("userToken") || "";
    if (token) {
      isLoggedIn = true;
      username = sessionStorage.getItem("username") || "";
    } else {
      isLoggedIn = false;
      username = "";
    }
  }
  
  // Handle search results when query changes
  $: if (searchQuery.trim()) {
    searchResults = allClasses.filter(cls =>
      cls.title.toLowerCase().includes(searchQuery.toLowerCase())
    );
  } else {
    searchResults = [];
  }

  // Initialize component
  onMount(() => {
    checkAuthStatus();
    if (isLoggedIn) {
      fetchClasses();
    }
  });
  
  // Re-check auth when URL changes
  $: if ($page.url) {
    checkAuthStatus();
  }

  // Toggle navbar collapse on mobile
  function handleUpdate(event: CustomEvent<boolean>) {
    isOpen = event.detail;
  }

  // Sign out user
  async function signOut() {
    try {
      if (token) {
        // Use foliumService to properly logout
        await foliumService.logout(token);
      }
      
      // Clear session storage
      if (browser) {
        sessionStorage.removeItem("userToken");
        sessionStorage.removeItem("username");
        sessionStorage.removeItem("userEmail");
        sessionStorage.removeItem("firstName");
        sessionStorage.removeItem("lastName");
      }
      
      isLoggedIn = false;
      goto("/log-in");
    } catch (error) {
      console.error("Error signing out:", error);
      // Still clear session and redirect even if API call fails
      if (browser) {
        sessionStorage.clear();
      }
      goto("/log-in");
    }
  }

  // Fetch classes for search functionality
  async function fetchClasses() {
    if (!token) return;
    
    try {
      const data = await foliumService.getMyClasses(token);
      allClasses = Object.values(data.classes).map((c: any) => ({
        id: c.id || c.classId,
        title: c.name || c.title || "Untitled Class"
      }));
    } catch (err) {
      console.error("Failed to fetch classes for search:", err);
    }
  }

  // Navigate to selected class page
  function selectClass(cls: { id: string; title: string }) {
    goto(`/classes/${cls.id}`);
    searchQuery = "";
  }
</script>

<Navbar color="secondary-subtle" light expand="md" container="md">
  <NavbarBrand href="/">Folium</NavbarBrand>

  <!-- Search box with dropdown results -->
  {#if isLoggedIn}
    <div class="search-container" style="position: relative;">
      <Input 
        type="text" 
        placeholder="Search classes" 
        class="mr-sm-2" 
        bind:value={searchQuery} 
      />
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
  {/if}
  
  <!-- Navbar Toggler for Mobile Views -->
  <NavbarToggler on:click={() => (isOpen = !isOpen)} />

  <!-- Collapsible Navbar -->
  <Collapse {isOpen} navbar expand="md" on:update={handleUpdate}>
    <Nav class="ms-auto" navbar>
      {#if isLoggedIn}
        <!-- Classes link for logged-in users -->
        <NavItem>
          <NavLink href="/classes" active={$page.url.pathname.startsWith('/classes')}>
            Classes
          </NavLink>
        </NavItem>
        
        <!-- User menu dropdown -->
        <Dropdown nav inNavbar>
          <DropdownToggle nav caret>
            {username || "Account"}
          </DropdownToggle>
          <DropdownMenu end>
            <DropdownItem href="/profile">Profile</DropdownItem>
            <DropdownItem divider />
            <DropdownItem on:click={signOut}>Sign Out</DropdownItem>
          </DropdownMenu>
        </Dropdown>
      {:else}
        <!-- Not logged in: Show Create Account + Log In -->
        <NavItem>
          <NavLink href="/register" active={$page.url.pathname === '/register'}>
            <Button color="primary">Create Account</Button>
          </NavLink>
        </NavItem>
        <NavItem>
          <NavLink href="/log-in" active={$page.url.pathname === '/log-in'}>
            <Button color="secondary">Log In</Button>
          </NavLink>
        </NavItem>
      {/if}
      
      <!-- Help & Resources dropdown -->
      <Dropdown nav inNavbar>
        <DropdownToggle nav caret>Help</DropdownToggle>
        <DropdownMenu end>
          <DropdownItem href="/help">Help Center</DropdownItem>
          <DropdownItem href="/about">About Folium</DropdownItem>
          <DropdownItem href="/contact">Contact Us</DropdownItem>
        </DropdownMenu>
      </Dropdown>
    </Nav>
  </Collapse>
</Navbar>

<style>
  .ms-auto {
    margin-left: auto;
  }

  /* Search box with dynamic dropdown */
  .search-container {
    width: 300px;
    position: relative;
    margin-right: 1rem;
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
    box-shadow: 0 4px 8px rgba(0,0,0,0.1);
  }

  .search-dropdown .dropdown-item {
    padding: 0.5rem 1rem;
    cursor: pointer;
    border-bottom: 1px solid #eee;
  }

  .search-dropdown .dropdown-item:hover {
    background-color: #f8f9fa;
  }
  
  /* Highlight active nav items */
  :global(.nav-link.active) {
    font-weight: bold;
    color: #007bff !important;
  }
  
  /* Make sure dropdowns appear above other content */
  :global(.dropdown-menu) {
    z-index: 1051;
  }
  
  @media (max-width: 768px) {
    .search-container {
      width: 100%;
      margin-bottom: 1rem;
      margin-right: 0;
    }
  }
</style>