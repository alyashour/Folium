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
  - `Token` (string, required): The refresh token issued during login.
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
    - `classes` (array): An array of class objects, each containing:
      - `classId` (string): The unique identifier of the class.
      - `name` (string): The name of the class.
      - `owner` (string): The username of the class owner.
      - `ownerId` (string): The user ID of the class owner.
      - `ownerContact` (string, optional): Contact information for the owner.
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
- **Description:** Gets the consolidated big note for a specific class.
- **Inputs:** None (uses authentication token and class ID from URL)
- **Outputs:**
  - **Success (200 OK):**
    - `bigNote` (object): The consolidated big note object containing:
      - `content` (string): The content of the big note.
      - `title` (string): The title of the big note.
      - `lastUpdated` (string): Timestamp of last update.
      - `contributors` (array): List of users who contributed to this note.
      - `sections` (array): Structured sections of the note.
  - **Error (401 Unauthorized):**
    - `error` (string): Authentication error message.
  - **Error (403 Forbidden):**
    - `error` (string): User doesn't have permission to access this class.
  - **Error (404 Not Found):**
    - `error` (string): Class not found or big note doesn't exist yet.

### POST /api/me/classes/{classId}/upload-note
- **Description:** Uploads a new note to be integrated into the class's big note.
- **Inputs:**
  - `noteFile` (multipart/form-data, required): The note file to upload.
  - `title` (string, optional): The title of the note.
- **Outputs:**
  - **Success (201 Created):**
    - `message` (string): Confirmation message.
    - `updated` (boolean): Whether the big note was updated.
    - `mergeConflicts` (array, optional): Any conflicts that occurred during merging.
  - **Error (400 Bad Request):**
    - `error` (string): Description of the validation error.
  - **Error (401 Unauthorized):**
    - `error` (string): Authentication error message.
  - **Error (403 Forbidden):**
    - `error` (string): User doesn't have permission to upload notes to this class.
  - **Error (404 Not Found):**
    - `error` (string): Class not found.

### PUT /api/me/classes/{classId}/bigNote/edit-note
- **Description:** Updates/edits the big note directly.
- **Inputs:**
  - `title` (string, optional): The updated title of the big note.
  - `content` (object): The updated content of the big note containing:
    - `sections` (array, optional): Updated sections.
    - `text` (string, optional): Raw text content.
- **Outputs:**
  - **Success (200 OK):**
    - `message` (string): Confirmation of successful update.
    - `lastUpdated` (string): New timestamp of the update.
  - **Error (400 Bad Request):**
    - `error` (string): Description of the validation error.
  - **Error (401 Unauthorized):**
    - `error` (string): Authentication error message.
  - **Error (403 Forbidden):**
    - `error` (string): User doesn't have permission to edit the big note.
  - **Error (404 Not Found):**
    - `error` (string): Class not found or big note doesn't exist yet.

### GET /api/me/classes/{classId}/bigNote/history
- **Description:** Gets the edit history of the big note.
- **Inputs:** None (uses authentication token and class ID from URL)
- **Outputs:**
  - **Success (200 OK):**
    - `history` (array): An array of edit events containing:
      - `timestamp` (string): When the edit occurred.
      - `userId` (string): Who made the edit.
      - `description` (string): Brief description of changes.
  - **Error (401 Unauthorized):**
    - `error` (string): Authentication error message.
  - **Error (403 Forbidden):**
    - `error` (string): User doesn't have permission to view this history.
  - **Error (404 Not Found):**
    - `error` (string): Class not found or big note doesn't exist yet.

### GET /api/me/classes/{classId}/bigNote/export
- **Description:** Exports the big note in a specified format.
- **Inputs:** 
  - `format` (string, optional): The format to export (PDF, Markdown, etc.). Defaults to PDF.
- **Outputs:**
  - **Success (200 OK):**
    - File download response with appropriate Content-Type header.
  - **Error (400 Bad Request):**
    - `error` (string): Invalid export format.
  - **Error (401 Unauthorized):**
    - `error` (string): Authentication error message.
  - **Error (403 Forbidden):**
    - `error` (string): User doesn't have permission to export this note.
  - **Error (404 Not Found):**
    - `error` (string): Class not found or big note doesn't exist yet.