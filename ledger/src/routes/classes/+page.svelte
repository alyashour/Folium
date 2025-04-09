<script lang="ts">
  import { Button, Alert, Badge } from '@sveltestrap/sveltestrap';
  import { onMount } from 'svelte';
  import foliumService from '../../lib/FoliumService'; // Adjusted the path
  import { goto } from '$app/navigation';
  
  let classes: Array<{ id: string; title: string; description?: string }> = [];
  let publicClasses: Array<{ id: string; title: string; owner?: string }> = [];
  let loading: boolean = true;
  let loadingPublic: boolean = false;
  let error: string | null = null;
  
  // Get token from session storage
  let token = '';

  // Modal states
  let showModal = false;
  let showEditModal = false;
  let showInfoModal = false;
  let showJoinModal = false;
  
  // Form data
  let className = "";
  let classDescription = "";
  let startDate = "";
  let endDate = "";
  
  // Edit class data
  let editingClass = { id: "", name: "", description: "" };
  let viewingClass = { id: "", title: "", description: "", owner: "" };
  
  // Search and filter
  let searchTerm = "";
  let showPublicClasses = false;
  
  onMount(() => {
    // Get token when component mounts
    token = sessionStorage.getItem('userToken') || '';
    fetchClasses();
  });
  
  // Fetch user's classes
  async function fetchClasses() {
    loading = true;
    error = null;
    try {
      // Use foliumService directly
      const data = await foliumService.getMyClasses(token);
      // Convert the object of classes into an array
      classes = Object.values(data.classes).map((c: any) => ({
        id: c.id || c.classId,
        title: c.name || c.title || "Untitled",
        description: c.description || ""
      }));
    } catch (err: any) {
      error = err.message;
    } finally {
      loading = false;
    }
  }
  
  // Fetch public classes
  async function fetchPublicClasses() {
    loadingPublic = true;
    try {
      // getAllClasses doesn't require authentication
      const data = await foliumService.getAllClasses();
      publicClasses = data.classes.map((c: any) => ({
        id: c.id || c.classId,
        title: c.name || c.title || "Untitled",
        owner: c.owner?.username || "Unknown"
      }));
    } catch (err: any) {
      console.error("Failed to fetch public classes:", err);
    } finally {
      loadingPublic = false;
    }
  }

  // Show/hide public classes
  function togglePublicClasses() {
    showPublicClasses = !showPublicClasses;
    if (showPublicClasses && publicClasses.length === 0) {
      fetchPublicClasses();
    }
  }
  
  // Functions to manage the modal popup
  const openModal = () => {
    showModal = true;
    className = "";
    classDescription = "";
    startDate = "";
    endDate = "";
  };

  const closeModal = () => {
    showModal = false;
  };

  // Open edit modal
  const openEditModal = async (classId: string) => {
    try {
      // Get class details for editing
      const classDetails = await foliumService.getClass(token, classId);
      editingClass = {
        id: classId,
        name: classDetails.name || classDetails.title || "",
        description: classDetails.description || ""
      };
      showEditModal = true;
    } catch (err: any) {
      alert(`Failed to get class details: ${err.message}`);
    }
  };

  // Close edit modal
  const closeEditModal = () => {
    showEditModal = false;
  };

  // Open class info modal
  const openInfoModal = async (classId: string) => {
    try {
      // Get class details and owner info
      const [classDetails, ownerInfo] = await Promise.all([
        foliumService.getClass(token, classId),
        foliumService.getClassOwner(token, classId)
      ]);
      
      viewingClass = {
        id: classId,
        title: classDetails.name || classDetails.title || "",
        description: classDetails.description || "",
        owner: ownerInfo?.username || "Unknown"
      };
      
      showInfoModal = true;
    } catch (err: any) {
      alert(`Failed to get class information: ${err.message}`);
    }
  };

  // Close class info modal
  const closeInfoModal = () => {
    showInfoModal = false;
  };

  // Create a new class
  const addClass = async () => {
    if (!className.trim()) {
      alert("Please enter a class name");
      return;
    }

    try {
      // Generate a unique classID
      const classID = `class-${Date.now()}`;
      
      // Create a description including dates if provided
      let description = classDescription || "";
      if (startDate || endDate) {
        if (description) description += "\n\n";
        description += `Class dates: ${startDate || 'N/A'} to ${endDate || 'N/A'}`;
      }
      
      // Use foliumService to create the class
      await foliumService.createClass(token, className, classID, description);
      
      // Refresh the class list
      await fetchClasses();
      
      // Close the modal
      closeModal();
    } catch (err: any) {
      alert(`Failed to create class: ${err.message}`);
    }
  };
  
  // Update an existing class
  const updateClass = async () => {
    try {
      await foliumService.updateClass(token, editingClass.id, {
        name: editingClass.name,
        description: editingClass.description
      });
      
      // Refresh the class list
      await fetchClasses();
      
      // Close the modal
      closeEditModal();
    } catch (err: any) {
      alert(`Failed to update class: ${err.message}`);
    }
  };
  
  // Delete a class
  const deleteClass = async (classId: string) => {
    if (confirm("Are you sure you want to delete this class? This action cannot be undone.")) {
      try {
        await foliumService.deleteClass(token, classId);
        await fetchClasses();
      } catch (err: any) {
        alert(`Failed to delete class: ${err.message}`);
      }
    }
  };
  
  // Navigate to class details page
  const viewClass = (classId: string) => {
    goto(`/classes/${classId}`);
  };
  
  // Filter classes based on search term
  $: filteredClasses = classes.filter(c => 
    c.title.toLowerCase().includes(searchTerm.toLowerCase()) ||
    (c.description && c.description.toLowerCase().includes(searchTerm.toLowerCase()))
  );
</script>

<h1>Your Classes</h1>

<div class="search-bar">
<input
  type="text"
  bind:value={searchTerm}
  placeholder="Search your classes..."
  class="search-input"
/>
<Button color="primary" class="ml-2" on:click={togglePublicClasses}>
  {showPublicClasses ? 'Hide Public Classes' : 'Show Public Classes'}
</Button>
<Button color="success" class="ml-2" on:click={openModal}>
  Create New Class
</Button>
</div>

{#if error}
<Alert color="danger" class="mt-3">
  Error loading your classes: {error}
  <Button color="link" on:click={fetchClasses}>Try Again</Button>
</Alert>
{/if}

{#if loading}
<div class="loading-spinner">
  <div class="spinner"></div>
  <p>Loading your classes...</p>
</div>
{:else if filteredClasses.length === 0 && !searchTerm}
<div class="empty-state">
  <h3>You don't have any classes yet</h3>
  <p>Create a new class to get started or join an existing one.</p>
  <Button color="primary" on:click={openModal}>Create Your First Class</Button>
</div>
{:else if filteredClasses.length === 0 && searchTerm}
<div class="empty-search">
  <p>No classes match your search for "{searchTerm}"</p>
  <Button color="link" on:click={() => searchTerm = ""}>Clear Search</Button>
</div>
{:else}
<h2 class="section-title">My Classes</h2>
<div class="tile-container">
  {#each filteredClasses as c}
    <div class="class-tile-wrapper">
      <div class="class-tile" on:click={() => viewClass(c.id)}>
        <h3>{c.title}</h3>
        {#if c.description}
          <p class="class-description">{c.description.length > 100 ? c.description.slice(0, 100) + '...' : c.description}</p>
        {/if}
        <div class="class-actions">
          <button class="action-button view-button" on:click|stopPropagation={() => viewClass(c.id)}>
            View
          </button>
          <button class="action-button info-button" on:click|stopPropagation={() => openInfoModal(c.id)}>
            Info
          </button>
          <button class="action-button edit-button" on:click|stopPropagation={() => openEditModal(c.id)}>
            Edit
          </button>
        </div>
      </div>
      <button class="delete-button" on:click|stopPropagation={() => deleteClass(c.id)}>×</button>
    </div>
  {/each}
</div>
{/if}

{#if showPublicClasses}
<h2 class="section-title mt-5">Public Classes</h2>

{#if loadingPublic}
  <div class="loading-spinner">
    <div class="spinner"></div>
    <p>Loading public classes...</p>
  </div>
{:else if publicClasses.length === 0}
  <div class="empty-state">
    <p>No public classes available</p>
  </div>
{:else}
  <div class="tile-container">
    {#each publicClasses as c}
      <div class="class-tile public-class">
        <h3>{c.title}</h3>
        <Badge color="info">Owner: {c.owner}</Badge>
        <div class="class-actions">
          <button class="action-button join-button">
            Join Class
          </button>
        </div>
      </div>
    {/each}
  </div>
{/if}
{/if}

<!-- Create Class Modal -->
{#if showModal}
<div class="custom-modal-overlay" on:click={closeModal}>
  <div class="custom-modal" on:click|stopPropagation>
    <div class="custom-modal-header">Create New Class</div>
    <div class="custom-modal-body">
      <div>
        <label for="className">Class Name: <span class="required">*</span></label>
        <input
          id="className"
          type="text"
          bind:value={className}
          placeholder="Enter class name"
          required
        />
      </div>
      <div>
        <label for="classDescription">Description:</label>
        <textarea
          id="classDescription"
          bind:value={classDescription}
          placeholder="Enter class description"
          rows="3"
        ></textarea>
      </div>
      <div class="modal-row">
        <div class="modal-column">
          <label for="startDate">Start Date:</label>
          <input
            id="startDate"
            type="date"
            bind:value={startDate}
          />
        </div>
        <div class="modal-column">
          <label for="endDate">End Date:</label>
          <input
            id="endDate"
            type="date"
            bind:value={endDate}
          />
        </div>
      </div>
    </div>
    <div class="custom-modal-footer">
      <button class="button-secondary" on:click={closeModal}>Cancel</button>
      <button class="button-primary" on:click={addClass}>Create Class</button>
    </div>
  </div>
</div>
{/if}

<!-- Edit Class Modal -->
{#if showEditModal}
<div class="custom-modal-overlay" on:click={closeEditModal}>
  <div class="custom-modal" on:click|stopPropagation>
    <div class="custom-modal-header">Edit Class</div>
    <div class="custom-modal-body">
      <div>
        <label for="editClassName">Class Name: <span class="required">*</span></label>
        <input
          id="editClassName"
          type="text"
          bind:value={editingClass.name}
          placeholder="Enter class name"
          required
        />
      </div>
      <div>
        <label for="editClassDescription">Description:</label>
        <textarea
          id="editClassDescription"
          bind:value={editingClass.description}
          placeholder="Enter class description"
          rows="3"
        ></textarea>
      </div>
    </div>
    <div class="custom-modal-footer">
      <button class="button-secondary" on:click={closeEditModal}>Cancel</button>
      <button class="button-primary" on:click={updateClass}>Save Changes</button>
    </div>
  </div>
</div>
{/if}

<!-- Class Info Modal -->
{#if showInfoModal}
<div class="custom-modal-overlay" on:click={closeInfoModal}>
  <div class="custom-modal" on:click|stopPropagation>
    <div class="custom-modal-header">Class Information</div>
    <div class="custom-modal-body">
      <div class="info-row">
        <strong>Title:</strong>
        <span>{viewingClass.title}</span>
      </div>
      {#if viewingClass.description}
        <div class="info-row">
          <strong>Description:</strong>
          <span>{viewingClass.description}</span>
        </div>
      {/if}
      <div class="info-row">
        <strong>Owner:</strong>
        <span>{viewingClass.owner}</span>
      </div>
    </div>
    <div class="custom-modal-footer">
      <button class="button-secondary" on:click={closeInfoModal}>Close</button>
      <button class="button-primary" on:click={() => viewClass(viewingClass.id)}>
        Open Class
      </button>
    </div>
  </div>
</div>
{/if}

<style>
h1 {
  margin-bottom: 1.5rem;
}

.section-title {
  font-size: 1.5rem;
  margin: 2rem 0 1rem 0;
  border-bottom: 2px solid #eee;
  padding-bottom: 0.5rem;
}

/* Search bar */
.search-bar {
  display: flex;
  align-items: center;
  margin-bottom: 1.5rem;
  flex-wrap: wrap;
  gap: 0.5rem;
}

.search-input {
  flex: 1;
  padding: 0.5rem;
  border: 1px solid #ddd;
  border-radius: 4px;
  min-width: 200px;
}

/* Loading spinner */
.loading-spinner {
  display: flex;
  flex-direction: column;
  align-items: center;
  padding: 2rem;
}

.spinner {
  border: 4px solid #f3f3f3;
  border-top: 4px solid #3498db;
  border-radius: 50%;
  width: 40px;
  height: 40px;
  animation: spin 1s linear infinite;
  margin-bottom: 1rem;
}

@keyframes spin {
  0% { transform: rotate(0deg); }
  100% { transform: rotate(360deg); }
}

/* Empty state */
.empty-state {
  text-align: center;
  padding: 3rem;
  border: 1px dashed #ddd;
  border-radius: 8px;
  background-color: #f9f9f9;
  margin: 2rem 0;
}

.empty-search {
  padding: 1.5rem;
  text-align: center;
  background-color: #f9f9f9;
  border-radius: 8px;
  margin: 1rem 0;
}

/* Flex container for the tiles */
.tile-container {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(300px, 1fr));
  gap: 1.25rem;
  margin-bottom: 2rem;
}

/* Class tile wrapper to contain both tile and delete button */
.class-tile-wrapper {
  position: relative;
}

/* Styling each tile */
.class-tile {
  display: block;
  color: inherit;
  border: 1px solid #ddd;
  border-radius: 8px;
  padding: 1rem;
  background-color: #f8f9fa;
  height: 100%;
  transition: transform 0.2s, background-color 0.2s;
  cursor: pointer;
}

.class-tile:hover {
  background-color: #e2e6ea;
  transform: translateY(-3px);
  box-shadow: 0 4px 8px rgba(0,0,0,0.1);
}

.class-tile h3 {
  margin: 0 0 0.5rem 0;
  font-size: 1.2rem;
}

.class-description {
  font-size: 0.9rem;
  color: #666;
  margin-bottom: 0.75rem;
}

.public-class {
  background-color: #f0f7ff;
  border-color: #bbd6ff;
}

/* Class actions */
.class-actions {
  display: flex;
  justify-content: flex-end;
  gap: 0.5rem;
  margin-top: 0.75rem;
}

.action-button {
  padding: 0.25rem 0.5rem;
  font-size: 0.875rem;
  border-radius: 4px;
  border: 1px solid #ddd;
  background-color: white;
  cursor: pointer;
  transition: background-color 0.2s;
}

.view-button {
  background-color: #e3f2fd;
  color: #0d6efd;
  border-color: #a2d4fd;
}

.edit-button {
  background-color: #fff8e1;
  color: #ffa000;
  border-color: #ffe082;
}

.info-button {
  background-color: #e8f5e9;
  color: #2e7d32;
  border-color: #a5d6a7;
}

.join-button {
  background-color: #e3f2fd;
  color: #0d6efd;
  border-color: #90caf9;
  padding: 0.25rem 0.75rem;
}

/* Delete button */
.delete-button {
  position: absolute;
  top: -10px;
  right: -10px;
  width: 24px;
  height: 24px;
  border-radius: 50%;
  background-color: #ff4d4f;
  color: white;
  border: none;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 16px;
  cursor: pointer;
  opacity: 0;
  transition: opacity 0.2s;
  z-index: 10;
}

.class-tile-wrapper:hover .delete-button {
  opacity: 1;
}

/* Modal styling */
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
  width: 450px;
  max-width: 95%;
  max-height: 90vh;
  overflow-y: auto;
  box-shadow: 0 2px 12px rgba(0, 0, 0, 0.4);
}

.custom-modal-header {
  font-size: 1.25rem;
  font-weight: bold;
  margin-bottom: 1rem;
  padding-bottom: 0.5rem;
  border-bottom: 1px solid #eee;
}

.custom-modal-body > div {
  margin-bottom: 0.75rem;
}

.custom-modal-body label {
  display: block;
  margin-bottom: 0.25rem;
  font-weight: 500;
}

.required {
  color: #ff4d4f;
}

.custom-modal-body input,
.custom-modal-body textarea {
  width: 100%;
  padding: 0.5rem;
  box-sizing: border-box;
  border: 1px solid #ddd;
  border-radius: 4px;
}

.custom-modal-body textarea {
  resize: vertical;
  min-height: 80px;
}

.modal-row {
  display: flex;
  gap: 1rem;
}

.modal-column {
  flex: 1;
}

.custom-modal-footer {
  display: flex;
  justify-content: flex-end;
  gap: 0.5rem;
  margin-top: 1.5rem;
  padding-top: 1rem;
  border-top: 1px solid #eee;
}

.button-primary {
  background-color: #0d6efd;
  color: white;
  border: none;
  padding: 0.5rem 1rem;
  border-radius: 4px;
  cursor: pointer;
}

.button-secondary {
  background-color: #f8f9fa;
  border: 1px solid #ddd;
  padding: 0.5rem 1rem;
  border-radius: 4px;
  cursor: pointer;
}

/* Info modal */
.info-row {
  margin-bottom: 0.75rem;
}

.info-row strong {
  display: block;
  margin-bottom: 0.25rem;
}
</style>