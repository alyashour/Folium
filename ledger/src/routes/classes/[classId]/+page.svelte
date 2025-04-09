<script lang="ts">
  import { onMount } from "svelte";
  import { goto } from "$app/navigation";
  import Note from "$lib/components/Note.svelte";
  import { fade } from 'svelte/transition';
  import foliumService from '../../../lib/FoliumService';
  import {
    Button,
    Col,
    Row,
    Dropdown,
    DropdownToggle,
    DropdownMenu,
    DropdownItem,
    Modal,
    ModalHeader,
    ModalBody,
    ModalFooter
  } from "@sveltestrap/sveltestrap";
  import { page } from "$app/stores";

  // Get the dynamic parameter (classId) from the URL.
  let classId: string;
  $: classId = $page.params.classId;
  
  // Get user token from session storage
  let token = '';
  
  // Note content and states
  let noteContent = '';
  let noteTitle = '';
  let noteLoading = false;
  let noteError: string | null = null;
  let noteHistory: any[] = [];
  let showHistory = false;

  // Appearance settings
  let textSizeDropdownOpen = false;
  let widthDropdownOpen = false;
  let colorDropdownOpen = false;
  let textSize = "medium";
  let colorMode: "light" | "dark" = "light";
  let widthMode: "wide" | "narrow" = "wide";

  // Modal states
  let joinModal = false;
  let joinDropdownOpen = false;
  let uploadModal = false;
  let exportModal = false;
  let historyModal = false;
  
  // Form data
  let selectedClassTitle = "";
  let joinMessage: string = "";
  let uploadFile: File | null = null;
  let uploadTitle: string = "";
  let exportFormat: string = "PDF";
  
  // Functions to toggle dropdowns
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

  // Initialize the page
  onMount(() => {
    token = sessionStorage.getItem('userToken') || '';
    fetchClasses();
    if (classId) {
      fetchNote(classId);
    }
  });
  
  // Fetch user's classes using foliumService
  async function fetchClasses() {
    classesLoading = true;
    classesError = null;
    
    try {
      const data = await foliumService.getMyClasses(token);
      classes = Object.values(data.classes).map((c: any) => ({
        id: c.id || c.classId,
        title: c.name || c.title || "Untitled"
      }));
    } catch (err: any) {
      classesError = err.message;
    } finally {
      classesLoading = false;
    }
  }
  
  // Refresh classes list
  function refreshClasses() {
    fetchClasses();
  }
  
  // Fetch the big note for a class
  async function fetchNote(classId: string) {
    if (!token || !classId) return;
    
    noteLoading = true;
    noteError = null;
    
    try {
      const data = await foliumService.getBigNote(token, classId);
      noteContent = data.content?.text || '';
      noteTitle = data.title || 'Untitled Note';
      
      // Also fetch history if showing history panel
      if (showHistory) {
        await fetchNoteHistory(classId);
      }
    } catch (err: any) {
      noteError = err.message;
    } finally {
      noteLoading = false;
    }
  }
  
  // Save note changes
  async function saveNote() {
    if (!token || !classId) return;
    
    try {
      await foliumService.editBigNote(token, classId, { text: noteContent }, noteTitle);
      // Show success message or update UI accordingly
    } catch (err: any) {
      // Show error message
      alert(`Failed to save note: ${err.message}`);
    }
  }
  
  // Fetch note edit history
  async function fetchNoteHistory(classId: string) {
    if (!token || !classId) return;
    
    try {
      const data = await foliumService.getNoteHistory(token, classId);
      noteHistory = data.history || [];
    } catch (err: any) {
      console.error("Failed to fetch note history:", err);
    }
  }
  
  // Upload a note file
  async function uploadNoteFile() {
    if (!token || !classId || !uploadFile) {
      alert("Missing required information");
      return;
    }
    
    try {
      await foliumService.uploadNote(token, classId, uploadFile, uploadTitle);
      uploadModal = false;
      uploadFile = null;
      uploadTitle = "";
      
      // Refresh the note content
      await fetchNote(classId);
    } catch (err: any) {
      alert(`Failed to upload note: ${err.message}`);
    }
  }
  
  // Export note to file
  async function exportNote() {
    if (!token || !classId) return;
    
    try {
      const blob = await foliumService.exportBigNote(token, classId, exportFormat);
      
      // Create download link
      const url = URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url;
      a.download = `${noteTitle || 'note'}.${exportFormat.toLowerCase()}`;
      document.body.appendChild(a);
      a.click();
      
      // Clean up
      document.body.removeChild(a);
      URL.revokeObjectURL(url);
      
      exportModal = false;
    } catch (err: any) {
      alert(`Failed to export note: ${err.message}`);
    }
  }
  
  // Handle file input change
  function handleFileInput(event: any) {
    const files = event.target.files;
    if (files && files.length > 0) {
      uploadFile = files[0];
    }
  }
  
  // Modal functions
  function openJoinModal() {
    joinModal = true;
  }
  
  function closeJoinModal() {
    joinModal = false;
    selectedClassTitle = "";
  }
  
  function openUploadModal() {
    uploadModal = true;
  }
  
  function closeUploadModal() {
    uploadModal = false;
    uploadFile = null;
    uploadTitle = "";
  }
  
  function openExportModal() {
    exportModal = true;
  }
  
  function closeExportModal() {
    exportModal = false;
  }
  
  function openHistoryModal() {
    fetchNoteHistory(classId);
    historyModal = true;
  }
  
  function closeHistoryModal() {
    historyModal = false;
  }
  
  // Toggle dropdown
  function toggleJoinDropdown() {
    joinDropdownOpen = !joinDropdownOpen;
  }
  
  // Function called when a class is selected from the dropdown
  function selectClass(cls: { id: string; title: string }) {
    selectedClassTitle = cls.title;
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
  
  // Appearance settings functions
  function setTextSize(size: string) {
    textSize = size;
  }
  
  function setColorMode(mode: "light" | "dark") {
    colorMode = mode;
  }
  
  function setWidthMode(mode: "wide" | "narrow") {
    widthMode = mode;
  }
  
  // Watch for changes in classId
  $: if (classId) {
    fetchNote(classId);
  }
</script>

<div class="content-container {colorMode} {textSize}">
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
      <div class="loading-spinner">
        <div class="spinner"></div>
      </div>
    {:else if classesError}
      <div class="alert alert-danger">
        <p>Error: {classesError}</p>
        <Button color="link" size="sm" on:click={fetchClasses}>Try Again</Button>
      </div>
    {:else if classes.length === 0}
      <p>No classes found.</p>
    {:else}
      <ul class="list-unstyled">
        {#each classes as c}
          <li>
            <a 
              href={`/classes/${c.id}`}
              class={c.id === classId ? 'active' : ''}
            >
              {c.title}
            </a>
          </li>
        {/each}
      </ul>
    {/if}

    <!-- Join Class button triggers the modal popup -->
    <Button color="primary" size="sm" on:click={openJoinModal} class="mt-3">Join Class</Button>

    <!-- Display the join message if available -->
    {#if joinMessage}
      <p class="join-message" transition:fade>{joinMessage}</p>
    {/if}
  </Col>

  <!-- Main Column: Note Component -->
  <Col md="8" class="note-container {widthMode}">
    {#if noteLoading}
      <div class="loading-spinner">
        <div class="spinner"></div>
        <p>Loading note...</p>
      </div>
    {:else if noteError}
      <div class="alert alert-danger">
        <p>Error loading note: {noteError}</p>
        <Button color="link" on:click={() => fetchNote(classId)}>Try Again</Button>
      </div>
    {:else}
      <div class="note-header">
        <input 
          type="text" 
          bind:value={noteTitle}
          class="note-title-input"
          placeholder="Note Title"
        />
        
        <div class="note-actions">
          <Button color="success" size="sm" on:click={saveNote} title="Save Note">
            Save
          </Button>
          <Button color="primary" size="sm" on:click={openUploadModal} title="Upload Note">
            Upload
          </Button>
          <Button color="info" size="sm" on:click={openExportModal} title="Export Note">
            Export
          </Button>
          <Button color="secondary" size="sm" on:click={openHistoryModal} title="View History">
            History
          </Button>
        </div>
      </div>
      
      <div class="note-editor">
        <textarea 
          bind:value={noteContent}
          class="note-content-textarea"
          placeholder="Start typing your note here..."
        ></textarea>
      </div>
    {/if}
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
        Width: {widthMode.charAt(0).toUpperCase() + widthMode.slice(1)}
      </DropdownToggle>
      <DropdownMenu>
        <DropdownItem on:click={() => setWidthMode('wide')}>Wide</DropdownItem>
        <DropdownItem on:click={() => setWidthMode('narrow')}>Narrow</DropdownItem>
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

<!-- Join Class Modal -->
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

<!-- Upload Note Modal -->
{#if uploadModal}
<div class="custom-modal-overlay" on:click={closeUploadModal}>
  <div class="custom-modal" on:click|stopPropagation>
    <div class="custom-modal-header">Upload Note</div>
    <div class="custom-modal-body">
      <div>
        <label for="uploadTitle">Note Title (Optional):</label>
        <input
          id="uploadTitle"
          type="text"
          bind:value={uploadTitle}
          placeholder="Enter note title"
        />
      </div>
      <div>
        <label for="uploadFile">Select File:</label>
        <input
          id="uploadFile"
          type="file"
          on:change={handleFileInput}
          accept=".pdf,.md,.txt"
        />
      </div>
      <div class="file-info">
        <small>Supported formats: PDF, Markdown, or text files</small>
      </div>
    </div>
    <div class="custom-modal-footer">
      <Button color="secondary" size="sm" on:click={closeUploadModal}>Cancel</Button>
      <Button 
        color="primary" 
        size="sm" 
        on:click={uploadNoteFile}
        disabled={!uploadFile}
      >
        Upload
      </Button>
    </div>
  </div>
</div>
{/if}

<!-- Export Note Modal -->
{#if exportModal}
<div class="custom-modal-overlay" on:click={closeExportModal}>
  <div class="custom-modal" on:click|stopPropagation>
    <div class="custom-modal-header">Export Note</div>
    <div class="custom-modal-body">
      <div>
        <label for="exportFormat">Export Format:</label>
        <select
          id="exportFormat"
          bind:value={exportFormat}
          class="form-select"
        >
          <option value="PDF">PDF</option>
          <option value="Markdown">Markdown</option>
          <option value="HTML">HTML</option>
          <option value="TXT">Plain Text</option>
        </select>
      </div>
    </div>
    <div class="custom-modal-footer">
      <Button color="secondary" size="sm" on:click={closeExportModal}>Cancel</Button>
      <Button color="primary" size="sm" on:click={exportNote}>Export</Button>
    </div>
  </div>
</div>
{/if}

<!-- Note History Modal -->
{#if historyModal}
<div class="custom-modal-overlay" on:click={closeHistoryModal}>
  <div class="custom-modal history-modal" on:click|stopPropagation>
    <div class="custom-modal-header">Note Edit History</div>
    <div class="custom-modal-body">
      {#if noteHistory.length === 0}
        <p>No edit history available for this note.</p>
      {:else}
        <div class="history-list">
          {#each noteHistory as entry}
            <div class="history-entry">
              <div class="history-timestamp">
                {new Date(entry.timestamp).toLocaleString()}
              </div>
              <div class="history-user">
                <strong>By:</strong> {entry.user || "Unknown"}
              </div>
              <div class="history-description">
                {entry.description || "Content updated"}
              </div>
            </div>
          {/each}
        </div>
      {/if}
    </div>
    <div class="custom-modal-footer">
      <Button color="primary" size="sm" on:click={closeHistoryModal}>Close</Button>
    </div>
  </div>
</div>
{/if}

<style>
/* Content container */
.content-container {
  min-height: 100vh;
  padding-bottom: 2rem;
}

/* Align header row items */
.header-row {
  margin-bottom: 1rem;
  display: flex;
  flex-direction: column;
}

/* Loading spinner */
.loading-spinner {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  padding: 2rem 0;
}

.spinner {
  border: 4px solid rgba(0, 0, 0, 0.1);
  border-radius: 50%;
  border-top: 4px solid #007bff;
  width: 30px;
  height: 30px;
  animation: spin 1s linear infinite;
}

@keyframes spin {
  0% { transform: rotate(0deg); }
  100% { transform: rotate(360deg); }
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

ul.list-unstyled li a.active {
  background-color: #007bff;
  color: white;
  font-weight: bold;
}

/* Header button styling */
.my-classes-header {
  font-weight: bold;
  text-transform: uppercase;
  padding: 0.75rem 1.5rem;
  border-radius: 50px;
}

/* Note editor */
.note-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-bottom: 1rem;
  flex-wrap: wrap;
  gap: 0.5rem;
}

.note-title-input {
  font-size: 1.5rem;
  padding: 0.5rem;
  border: 1px solid #ddd;
  border-radius: 4px;
  flex-grow: 1;
  min-width: 200px;
}

.note-actions {
  display: flex;
  gap: 0.5rem;
  flex-wrap: wrap;
}

.note-content-textarea {
  width: 100%;
  min-height: 400px;
  padding: 1rem;
  border: 1px solid #ddd;
  border-radius: 4px;
  font-family: 'Courier New', monospace;
  resize: vertical;
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

.history-modal {
  width: 500px;
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

.custom-modal-body input,
.custom-modal-body select {
  width: 100%;
  padding: 0.5rem;
  box-sizing: border-box;
  border: 1px solid #ddd;
  border-radius: 4px;
}

.file-info {
  color: #666;
}

.custom-modal-footer {
  display: flex;
  justify-content: flex-end;
  gap: 0.5rem;
  margin-top: 1rem;
}

/* History list */
.history-list {
  max-height: 300px;
  overflow-y: auto;
}

.history-entry {
  border-bottom: 1px solid #eee;
  padding-bottom: 0.75rem;
  margin-bottom: 0.75rem;
}

.history-timestamp {
  font-weight: bold;
  color: #007bff;
}

.history-entry:last-child {
  border-bottom: none;
}

/* Join message */
.join-message {
  margin-top: 0.5rem;
  font-size: 0.9rem;
  color: #007bff;
}

/* Text size variations */
.content-container.small {
  font-size: 0.85rem;
}

.content-container.medium {
  font-size: 1rem;
}

.content-container.large {
  font-size: 1.15rem;
}

/* Color modes */
.content-container.light {
  background-color: #fff;
  color: #000;
}

.content-container.dark {
  background-color: #222;
  color: #fff;
}

.content-container.dark .note-content-textarea,
.content-container.dark .note-title-input {
  background-color: #333;
  color: #fff;
  border-color: #555;
}

.content-container.dark ul.list-unstyled li a:not(.active) {
  background-color: #333;
  color: #72b4ff;
  border-color: #555;
}

/* Width variations */
.note-container.wide {
  max-width: 100%;
}

.note-container.narrow {
  max-width: 60%;
  margin: 0 auto;
}
</style>