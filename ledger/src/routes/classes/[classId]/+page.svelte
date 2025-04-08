<script lang="ts">
    import { onMount } from "svelte";
    import { PUBLIC_API_BASE_URL } from "$env/static/public";
    import Note from "$lib/components/Note.svelte";
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
  
    function refreshClasses() {
      fetchClasses();
    }
  </script>
  
  
  <Row class="mt-3">
    <!-- Left Column: Class Selector -->
    <Col md="2">
      <div class="header-row d-flex align-items-center justify-content-between">
        <h5 class="mb-0"><a href="/classes">My Classes</a></h5>
        <Button color="secondary" size="sm" on:click={refreshClasses} title="Refresh">
          🔄
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
      <Button color="primary" size="sm">New Class</Button>
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
          Text Size
        </DropdownToggle>
        <DropdownMenu>
          <DropdownItem>Small</DropdownItem>
          <DropdownItem>Medium</DropdownItem>
          <DropdownItem>Large</DropdownItem>
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
          <DropdownItem>Automatic</DropdownItem>
          <DropdownItem>Light</DropdownItem>
          <DropdownItem>Dark</DropdownItem>
        </DropdownMenu>
      </Dropdown>
    </Col>
  </Row>
  
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
  </style>
  