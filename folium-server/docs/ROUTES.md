# API Routes

## Authentication Routes

### POST /api/auth/register
- **Description:** Registers a new user.
- **Inputs:**
  - `username` (string, required): The username for the new user.
  - `password` (string, required): The password for the new user.
- **Outputs:**
  - **Success (201 Created):**
    - `message` (string): Confirmation message.
    - `userId` (integer): The ID of the newly created user.
  - **Error (400 Bad Request):**
    - `error` (string): Description of the validation error.

### POST /api/auth/login
- **Description:** Authenticates a user and starts a session.
- **Inputs:**
  - `username` (string, required): The username of the user.
  - `password` (string, required): The password of the user.
- **Outputs:**
  - **Success (200 OK):**
    - `token` (string): A JSON Web Token for session management.
    - `sessionId` (string): The session identifier.
  - **Error (401 Unauthorized):**
    - `error` (string): Invalid credentials message.

### POST /api/auth/logout
- **Description:** Logs out the user and ends the session.
- **Inputs:**
  - `token` (string, required): The JSON Web Token of the user.
- **Outputs:**
  - **Success (200 OK):**
    - `message` (string): Confirmation of successful logout.
  - **Error (401 Unauthorized):**
    - `error` (string): Invalid or expired token message.

### POST /api/auth/refresh-token
- **Description:** Refreshes the user's authentication token.
- **Inputs:**
  - `refreshToken` (string, required): The refresh token issued during login.
- **Outputs:**
  - **Success (200 OK):**
    - `token` (string): A new JSON Web Token for session management.
    - `refreshToken` (string): A new refresh token.
  - **Error (401 Unauthorized):**
    - `error` (string): Invalid or expired refresh token message.

### POST /api/auth/change-password
- **Description:** Allows a user to change their password.
- **Inputs:**
  - `currentPassword` (string, required): The user's current password.
  - `newPassword` (string, required): The new password to set.
- **Outputs:**
  - **Success (200 OK):**
    - `message` (string): Confirmation of successful password change.
  - **Error (400 Bad Request):**
    - `error` (string): Validation error or incorrect current password.

## Classes Routes

### GET /api/classes
- **Description:** Gets a list of all class IDs (global route, no permissions required).
- **Inputs:** None
- **Outputs:**
  - **Success (200 OK):**
    - `classes` (array): An array of class IDs.
    - array of objects ( classID, , owner, ownerID, ownersContact)
  - **Error (500 Internal Server Error):**
    - `error` (string): Description of the server error.

### GET /api/me/classes
- **Description:** Gets a list of classes that the authenticated user is enrolled in or owns.
- **Inputs:** None (uses authentication token)
- **Outputs:**
  - **Success (200 OK):**
    - `classes` (array): An array of class objects.
      - Each object contains:
        - `id` (string): The class ID.
        - `owner` (string): The owner's user ID. 
        - `name` (string): The class name.
  - **Error (401 Unauthorized):**
    - `error` (string): Authentication error message.

### POST /api/me/classes
- **Description:** Creates a new class for the authenticated user.
- **Inputs:**
  - `name` (string, required): The name of the class.
  - classID (string required): class id
  - `description` (string, optional): A description of the class.
- **Outputs:**
  - **Success (201 Created):**
    - `message` (string): Confirmation message.
    - `classId` (string): The ID of the newly created class.
  - **Error (400 Bad Request):**
    - `error` (string): Description of the validation error.
  - **Error (401 Unauthorized):**
    - `error` (string): Authentication error message.

### PUT /api/me/classes/{classId}
- **Description:** Updates a class that the authenticated user owns.
- **Inputs:**
  - `name` (string, optional): The updated name of the class.
  - `description` (string, optional): The updated description of the class.
- **Outputs:**
  - **Success (200 OK):**
    - `message` (string): Confirmation of successful update.
  - **Error (400 Bad Request):**
    - `error` (string): Description of the validation error.
  - **Error (401 Unauthorized):**
    - `error` (string): Authentication error message.
  - **Error (403 Forbidden):**
    - `error` (string): User doesn't have permission to update this class.
  - **Error (404 Not Found):**
    - `error` (string): Class not found.
 
### DELETE /api/me/classes/{classId}
- **Description:** Removes a class that the authenticated user owns or removes the user from an enrolled class.
- **Inputs:** None (uses authentication token and class ID from URL)
- **Outputs:**
  - **Success (200 OK):**
    - `message` (string): Confirmation of successful deletion or removal.
  - **Error (401 Unauthorized):**
    - `error` (string): Authentication error message.
  - **Error (403 Forbidden):**
    - `error` (string): User doesn't have permission to delete this class.
  - **Error (404 Not Found):**
    - `error` (string): Class not found.

### GET /api/me/classes/{classId}
- **Description:** Gets detailed information about a specific class.
- **Inputs:** None (uses authentication token and class ID from URL)
- **Outputs:**
  - **Success (200 OK):**
    - `id` (string): The class ID.
    - `owner` (string): The owner's user ID.
    - `name` (string): The class name.
    - `description` (string): The class description.
    - `bigNote` (string): The consolidated big note for the class.
    - `title` (string): The title of the class.
  - **Error (401 Unauthorized):**
    - `error` (string): Authentication error message.
  - **Error (403 Forbidden):**
    - `error` (string): User doesn't have permission to access this class.
  - **Error (404 Not Found):**
    - `error` (string): Class not found.

### GET /api/me/classes/{classId}/owner
- **Description:** Gets the owner information for a specific class.
- **Inputs:** None (uses authentication token and class ID from URL)
- **Outputs:**
  - **Success (200 OK):**
    - `ownerId` (string): The owner's user ID.
    - `ownerName` (string): The owner's username.
    - `ownerContact` (string, optional): Contact information for the owner.
  - **Error (401 Unauthorized):**
    - `error` (string): Authentication error message.
  - **Error (403 Forbidden):**
    - `error` (string): User doesn't have permission to access this class.
  - **Error (404 Not Found):**
    - `error` (string): Class not found.

### GET /api/me/classes/{classId}/name
- **Description:** Gets the name of a specific class.
- **Inputs:** None (uses authentication token and class ID from URL)
- **Outputs:**
  - **Success (200 OK):**
    - `name` (string): The class name.
  - **Error (401 Unauthorized):**
    - `error` (string): Authentication error message.
  - **Error (403 Forbidden):**
    - `error` (string): User doesn't have permission to access this class.
  - **Error (404 Not Found):**
    - `error` (string): Class not found.

### GET /api/me/classes/{classId}/description
- **Description:** Gets the description of a specific class.
- **Inputs:** None (uses authentication token and class ID from URL)
- **Outputs:**
  - **Success (200 OK):**
    - `description` (string): The class description.
  - **Error (401 Unauthorized):**
    - `error` (string): Authentication error message.
  - **Error (403 Forbidden):**
    - `error` (string): User doesn't have permission to access this class.
  - **Error (404 Not Found):**
    - `error` (string): Class not found.

### GET /api/me/classes/{classId}/bigNote
- **Description:** Gets the consolidated big note for a specific class.
- **Inputs:** None (uses authentication token and class ID from URL)
- **Outputs:**
  - **Success (200 OK):**
    - `bigNote` (string): The consolidated big note content.
    - `lastUpdated` (string): Timestamp of last update.
  - **Error (401 Unauthorized):**
    - `error` (string): Authentication error message.
  - **Error (403 Forbidden):**
    - `error` (string): User doesn't have permission to access this class.
  - **Error (404 Not Found):**
    - `error` (string): Class not found or big note doesn't exist yet.

### GET /api/me/classes/{classId}/title
- **Description:** Gets the title of a specific class.
- **Inputs:** None (uses authentication token and class ID from URL)
- **Outputs:**
  - **Success (200 OK):**
    - `title` (string): The class title.
  - **Error (401 Unauthorized):**
    - `error` (string): Authentication error message.
  - **Error (403 Forbidden):**
    - `error` (string): User doesn't have permission to access this class.
  - **Error (404 Not Found):**
    - `error` (string): Class not found.

## Notes Routes

### GET /api/me/classes/{classId}/bigNote
- **Description:** Gets a list of notes for a specific class.
- **Inputs:** None (uses authentication token and class ID from URL)
- **Outputs:**
this will be one bigNote
  - **Success (200 OK):**
    - `notes` (array): An array of note objects. 
      - Each note object contains:
        - `id` (string): The note ID.
        - `title` (string): The note title.
        - `createdAt` (string): The creation timestamp.
        - `updatedAt` (string): The last update timestamp.
  - **Error (401 Unauthorized):**
    - `error` (string): Authentication error message.
  - **Error (403 Forbidden):**
    - `error` (string): User doesn't have permission to access this class.
  - **Error (404 Not Found):**
    - `error` (string): Class not found.

### POST /api/me/classes/{classId}/upload-note
- **Description:** Uploads a new note to a specific class.
- **Inputs:**
  - `noteFile` (multipart/form-data, required): The note file to upload.
  - `title` (string, optional): The title of the note.
- **Outputs:**
  - **Success (201 Created):**
    - `message` (string): Confirmation message.
    - `noteId` (string): The ID of the newly created note.
  - **Error (400 Bad Request):**
    - `error` (string): Description of the validation error.
  - **Error (401 Unauthorized):**
    - `error` (string): Authentication error message.
  - **Error (403 Forbidden):**
    - `error` (string): User doesn't have permission to upload notes to this class.
  - **Error (404 Not Found):**
    - `error` (string): Class not found.

<!-- ### GET /api/me/classes/{classId}/notes/{noteId}
- **Description:** Gets the content and metadata of a specific note.
- **Inputs:** None (uses authentication token, class ID, and note ID from URL)
- **Outputs:**
  - **Success (200 OK):**
    - `id` (string): The note ID.
    - `title` (string): The note title.
    - `content` (string): The content of the note.
    - `structuredContent` (object): The structured representation of the note.
    - `createdAt` (string): The creation timestamp.
    - `updatedAt` (string): The last update timestamp.
  - **Error (401 Unauthorized):**
    - `error` (string): Authentication error message.
  - **Error (403 Forbidden):**
    - `error` (string): User doesn't have permission to access this note.
  - **Error (404 Not Found):**
    - `error` (string): Note or class not found. -->

edit a specfic note 
### PUT /api/me/classes/{classId}/bigNote/edit-note
- **Description:** Updates/edits an existing note.
- **Inputs:**
  - `title` (string, optional): The updated title of the note.
  - `content` (json object): The updated content of the note.
- **Outputs:**
  - **Success (200 OK):**
    - `message` (string): Confirmation of successful update.
  - **Error (400 Bad Request):**
    - `error` (string): Description of the validation error.
  - **Error (401 Unauthorized):**
    - `error` (string): Authentication error message.
  - **Error (403 Forbidden):**
    - `error` (string): User doesn't have permission to edit this note.
  - **Error (404 Not Found):**
    - `error` (string): Note or class not found.

### DELETE /api/me/classes/{classId}/notes/{noteId}
- **Description:** Deletes a note.
- **Inputs:** None (uses authentication token, class ID, and note ID from URL)
- **Outputs:**
  - **Success (200 OK):**
    - `message` (string): Confirmation of successful deletion.
  - **Error (401 Unauthorized):**
    - `error` (string): Authentication error message.
  - **Error (403 Forbidden):**
    - `error` (string): User doesn't have permission to delete this note.
  - **Error (404 Not Found):**
    - `error` (string): Note or class not found.