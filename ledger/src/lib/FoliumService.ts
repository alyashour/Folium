const API_BASE = import.meta.env.VITE_PUBLIC_API_BASE_URL;

/**
 * Helper function to process API responses
 * 
 * @param res - The Response object from fetch
 * @returns The parsed JSON data from the response
 * @throws Error with status code and error message if response is not OK
 * 
 * This function handles the common pattern of checking if an HTTP response 
 * was successful (status in the 200-299 range) and either:
 * 1. Parsing and returning the JSON response data if successful
 * 2. Throwing an error with the HTTP status code and response text if not successful
 */
async function handleResponse(res: Response) {
  if (!res.ok) {
    const errorText = await res.text();
    throw new Error(`HTTP ${res.status}: ${errorText}`);
  }
  return res.json();
}

class FoliumService {
  // === AUTH ===
  
  /**
   * Registers a new user in the system
   * 
   * @param username - The desired username for registration
   * @param password - The password for the new account
   * @returns A promise that resolves to the registration response object containing
   *          userId and confirmation message
   * @throws Error if registration fails (e.g., username already exists)
   */
  async register(username: string, password: string) {
    const res = await fetch(`${API_BASE}/api/auth/register`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ username, password })
    });
    return handleResponse(res);
  }
  
  /**
   * Authenticates a user and establishes a session
   * 
   * @param username - The user's username
   * @param password - The user's password
   * @returns A promise that resolves to the login response containing authentication token
   *          and session information when successful
   * @throws Error if authentication fails (e.g., invalid credentials)
   */
  async login(username: string, password: string) {
    const res = await fetch(`${API_BASE}/api/auth/login`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ username, password })
    });
    return handleResponse(res);
  }
  
  /**
   * Logs out the current user and terminates their session
   * 
   * @param token - The authentication token to invalidate
   * @returns A promise that resolves to the logout confirmation response
   * @throws Error if logout fails (e.g., invalid token)
   */
  async logout(token: string) {
    const res = await fetch(`${API_BASE}/api/auth/logout`, {
      method: 'POST',
      headers: { 
        'Content-Type': 'application/json',
        'Authorization': `Bearer ${token}`
      },
      body: JSON.stringify({ token })
    });
    return handleResponse(res);
  }
  
  /**
   * Refreshes an expired authentication token using a refresh token
   * 
   * @param refreshToken - The refresh token obtained during login
   * @returns A promise that resolves to an object containing a new authentication token
   *          and a new refresh token
   * @throws Error if token refresh fails (e.g., invalid or expired refresh token)
   */
  async refreshToken(refreshToken: string) {
    const res = await fetch(`${API_BASE}/api/auth/refresh-token`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ refreshToken })
    });
    return handleResponse(res);
  }
  
  /**
   * Changes the password for the currently authenticated user
   * 
   * @param currentPassword - The user's current password for verification
   * @param newPassword - The new password to set
   * @returns A promise that resolves to the password change confirmation response
   * @throws Error if password change fails (e.g., incorrect current password)
   */
  async changePassword(currentPassword: string, newPassword: string) {
    const res = await fetch(`${API_BASE}/api/auth/change-password`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ currentPassword, newPassword })
    });
    return handleResponse(res);
  }

  // === CLASSES ===
  
  /**
   * Retrieves a list of all available classes in the system
   * 
   * @returns A promise that resolves to an array of class objects, each containing
   *          basic information like classId, name, owner, and contact details
   * @throws Error if the server request fails
   * 
   * This is a public endpoint that doesn't require authentication
   */
  async getAllClasses() {
    const res = await fetch(`${API_BASE}/api/classes`);
    return handleResponse(res);
  }

  /**
   * Retrieves all classes that the authenticated user is enrolled in or owns
   * 
   * @param token - The authentication token of the user
   * @returns A promise that resolves to an array of class objects associated with the user
   * @throws Error if authentication fails or if the server request fails
   */
  async getMyClasses(token: string) {
    const res = await fetch(`${API_BASE}/api/me/classes`, {
      headers: {
        'Authorization': `Bearer ${token}`
      }
    });
    return handleResponse(res);
  }

  /**
   * Creates a new class with the authenticated user as the owner
   * 
   * @param token - The authentication token of the user
   * @param name - The name of the class to create
   * @param classID - The unique identifier for the class
   * @param description - An optional description of the class
   * @returns A promise that resolves to the class creation response containing
   *          confirmation message and the ID of the newly created class
   * @throws Error if authentication fails, if class creation fails (e.g., duplicate ID),
   *         or if the server request fails
   */
  async createClass(token: string, name: string, classID: string, description?: string) {
    const res = await fetch(`${API_BASE}/api/me/classes`, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
        'Authorization': `Bearer ${token}`
      },
      body: JSON.stringify({ name, classID, description })
    });
    return handleResponse(res);
  }

  /**
   * Updates an existing class's information
   * 
   * @param token - The authentication token of the user
   * @param classId - The ID of the class to update
   * @param updates - An object containing the fields to update (name and/or description)
   * @returns A promise that resolves to the update confirmation response
   * @throws Error if authentication fails, if the user doesn't have permission to update the class,
   *         if the class doesn't exist, or if the server request fails
   */
  async updateClass(token: string, classId: string, updates: { name?: string; description?: string }) {
    const res = await fetch(`${API_BASE}/api/me/classes/${classId}`, {
      method: 'PUT',
      headers: {
        'Content-Type': 'application/json',
        'Authorization': `Bearer ${token}`
      },
      body: JSON.stringify(updates)
    });
    return handleResponse(res);
  }

  /**
   * Deletes a class or removes the user from an enrolled class
   * 
   * @param token - The authentication token of the user
   * @param classId - The ID of the class to delete or leave
   * @returns A promise that resolves to the deletion confirmation response
   * @throws Error if authentication fails, if the user doesn't have permission to delete the class,
   *         if the class doesn't exist, or if the server request fails
   */
  async deleteClass(token: string, classId: string) {
    const res = await fetch(`${API_BASE}/api/me/classes/${classId}`, {
      method: 'DELETE',
      headers: {
        'Authorization': `Bearer ${token}`
      }
    });
    return handleResponse(res);
  }

  /**
   * Retrieves detailed information about a specific class
   * 
   * @param token - The authentication token of the user
   * @param classId - The ID of the class to retrieve
   * @returns A promise that resolves to the class details including ID, owner, name, description,
   *          big note content, and title
   * @throws Error if authentication fails, if the user doesn't have permission to access the class,
   *         if the class doesn't exist, or if the server request fails
   */
  async getClass(token: string, classId: string) {
    const res = await fetch(`${API_BASE}/api/me/classes/${classId}`, {
      headers: {
        'Authorization': `Bearer ${token}`
      }
    });
    return handleResponse(res);
  }

  /**
   * Retrieves information about the owner of a specific class
   * 
   * @param token - The authentication token of the user
   * @param classId - The ID of the class
   * @returns A promise that resolves to the owner information including ID, name, and contact details
   * @throws Error if authentication fails, if the user doesn't have permission to access this information,
   *         if the class doesn't exist, or if the server request fails
   */
  async getClassOwner(token: string, classId: string) {
    const res = await fetch(`${API_BASE}/api/me/classes/${classId}/owner`, {
      headers: {
        'Authorization': `Bearer ${token}`
      }
    });
    return handleResponse(res);
  }

  /**
   * Retrieves the name of a specific class
   * 
   * @param token - The authentication token of the user
   * @param classId - The ID of the class
   * @returns A promise that resolves to an object containing the class name
   * @throws Error if authentication fails, if the user doesn't have permission to access this information,
   *         if the class doesn't exist, or if the server request fails
   */
  async getClassName(token: string, classId: string) {
    const res = await fetch(`${API_BASE}/api/me/classes/${classId}/name`, {
      headers: {
        'Authorization': `Bearer ${token}`
      }
    });
    return handleResponse(res);
  }

  /**
   * Retrieves the description of a specific class
   * 
   * @param token - The authentication token of the user
   * @param classId - The ID of the class
   * @returns A promise that resolves to an object containing the class description
   * @throws Error if authentication fails, if the user doesn't have permission to access this information,
   *         if the class doesn't exist, or if the server request fails
   */
  async getClassDescription(token: string, classId: string) {
    const res = await fetch(`${API_BASE}/api/me/classes/${classId}/description`, {
      headers: {
        'Authorization': `Bearer ${token}`
      }
    });
    return handleResponse(res);
  }

  /**
   * Retrieves the title of a specific class
   * 
   * @param token - The authentication token of the user
   * @param classId - The ID of the class
   * @returns A promise that resolves to an object containing the class title
   * @throws Error if authentication fails, if the user doesn't have permission to access this information,
   *         if the class doesn't exist, or if the server request fails
   */
  async getClassTitle(token: string, classId: string) {
    const res = await fetch(`${API_BASE}/api/me/classes/${classId}/title`, {
      headers: {
        'Authorization': `Bearer ${token}`
      }
    });
    return handleResponse(res);
  }


     // === NOTES ===
  
  /**
   * Retrieves the consolidated big note for a specific class
   * 
   * @param token - The authentication token of the user
   * @param classId - The ID of the class
   * @returns A promise that resolves to the big note object containing content, title, contributors,
   *          and other metadata
   * @throws Error if authentication fails, if the user doesn't have permission to access this information,
   *         if the class doesn't exist, or if the server request fails
   */
  async getBigNote(token: string, classId: string) {
    const res = await fetch(`${API_BASE}/api/me/classes/${classId}/bigNote`, {
      headers: {
        'Authorization': `Bearer ${token}`
      }
    });
    return handleResponse(res);
  }

  /**
   * Uploads a new note to be integrated into a class's big note
   * 
   * @param token - The authentication token of the user
   * @param classId - The ID of the class to add the note to
   * @param noteFile - The note file to upload (PDF, Markdown, or text format)
   * @param title - Optional title for the note
   * @returns A promise that resolves to the upload response containing confirmation message
   *          and information about how the note was integrated
   * @throws Error if authentication fails, if the user doesn't have permission to upload to this class,
   *         if the file format is invalid, or if the server request fails
   */
  async uploadNote(token: string, classId: string, noteFile: File, title?: string) {
    const formData = new FormData();
    formData.append('noteFile', noteFile);
    if (title) formData.append('title', title);

    const res = await fetch(`${API_BASE}/api/me/classes/${classId}/upload-note`, {
      method: 'POST',
      headers: {
        'Authorization': `Bearer ${token}`
      },
      body: formData
    });
    return handleResponse(res);
  }

  /**
   * Directly edits the content of a class's big note
   * 
   * @param token - The authentication token of the user
   * @param classId - The ID of the class whose big note is being edited
   * @param content - The updated content object containing sections and/or raw text
   * @param title - Optional new title for the big note
   * @returns A promise that resolves to the edit response containing confirmation message
   *          and timestamp of the update
   * @throws Error if authentication fails, if the user doesn't have permission to edit this note,
   *         if the content format is invalid, if the class doesn't exist, or if the server request fails
   */
  async editBigNote(token: string, classId: string, content: { sections?: any[]; text?: string }, title?: string) {
    const body = {
      content,
      ...(title && { title })
    };

    const res = await fetch(`${API_BASE}/api/me/classes/${classId}/bigNote/edit-note`, {
      method: 'PUT',
      headers: {
        'Content-Type': 'application/json',
        'Authorization': `Bearer ${token}`
      },
      body: JSON.stringify(body)
    });
    return handleResponse(res);
  }

  /**
   * Retrieves the edit history of a class's big note
   * 
   * @param token - The authentication token of the user
   * @param classId - The ID of the class whose note history is being requested
   * @returns A promise that resolves to an array of historical edit events, each containing
   *          timestamp, user information, and description of changes
   * @throws Error if authentication fails, if the user doesn't have permission to view this history,
   *         if the class doesn't exist, or if the server request fails
   */
  async getNoteHistory(token: string, classId: string) {
    const res = await fetch(`${API_BASE}/api/me/classes/${classId}/bigNote/history`, {
      headers: {
        'Authorization': `Bearer ${token}`
      }
    });
    return handleResponse(res);
  }

  /**
   * Exports a class's big note in the specified format for download
   * 
   * @param token - The authentication token of the user
   * @param classId - The ID of the class whose big note is being exported
   * @param format - The desired export format (defaults to "PDF", other options may include "Markdown", "HTML", etc.)
   * @returns A promise that resolves to a Blob object containing the exported file data
   * @throws Error if authentication fails, if the user doesn't have permission to export this note,
   *         if the requested format is invalid, if the class doesn't exist, or if the server request fails
   * 
   * Note: Unlike other methods that return JSON, this method returns a raw Blob for file download
   */
  async exportBigNote(token: string, classId: string, format = "PDF") {
    const res = await fetch(`${API_BASE}/api/me/classes/${classId}/bigNote/export?format=${format}`, {
      headers: {
        'Authorization': `Bearer ${token}`
      }
    });

    if (!res.ok) {
      const errorText = await res.text();
      throw new Error(`HTTP ${res.status}: ${errorText}`);
    }

    // Return raw blob for download
    return res.blob();
  }
}


const foliumService = new FoliumService();
export default foliumService;