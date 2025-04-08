"""
Prototype Python Server for Folium

Endpoints:
  - /ping: Health check endpoint.
  - /api/auth/register: Register a new user.
  - /api/auth/login: Login an existing user and return a dummy token.
  - /api/auth/validate: Validate an existing token (expects the token in the Authorization header as "Bearer <token>").
  - /api/auth/refresh: Refresh an expired token.
  - /api/auth/change-password: Change a user's password.
  - /api/me/classes: Return available class information.

The server seeds a default user ("admin" with password "password") if not already present.
"""

from flask import Flask, request, jsonify, abort
from flask_cors import CORS

app = Flask(__name__)
CORS(app)  # Allow cross-origin requests for testing

# In-memory user "database"
users = {}

def seed_default_user():
    """Seed with default user if not already present."""
    if "admin" not in users:
        # Default password is "password"; in production, use proper password hashing!
        users["admin"] = {"password": "password", "role": "admin"}
        print("Default user 'admin' seeded.")

seed_default_user()

@app.route("/ping", methods=["GET"])
def ping():
    """Health check endpoint."""
    return jsonify(status="ok", message="pong"), 200

@app.route("/api/auth/register", methods=["POST"])
def register():
    """Register a new user.
       Expects JSON with 'username' and 'password'.
    """
    data = request.get_json()
    if not data:
        abort(400, description="Invalid JSON")
    username = data.get("username")
    password = data.get("password")
    if not username or not password:
        abort(400, description="Missing username or password")
    if username in users:
        return jsonify({"status": "error", "message": "User already exists"}), 409
    users[username] = {"password": password, "role": "user"}
    return jsonify({"status": "ok", "message": f"User '{username}' registered successfully"}), 201

@app.route("/api/auth/login", methods=["POST"])
def login():
    """Login an existing user.
       Expects JSON with 'username' and 'password'.
       Returns a dummy token if successful.
    """
    data = request.get_json()
    if not data:
        abort(400, description="Invalid JSON")
    username = data.get("username")
    password = data.get("password")
    if not username or not password:
        abort(400, description="Missing credentials")
    
    user = users.get(username)
    if user and user["password"] == password:
        # In a real application, generate and sign a proper JWT token.
        token = f"token_for_{username}"
        return jsonify({"status": "ok", "token": token, "role": user["role"]}), 200
    else:
        return jsonify({"status": "error", "message": "Invalid credentials"}), 401

@app.route("/api/auth/validate", methods=["GET"])
def validate_token():
    """
    Validate an authentication token.
    Expects the token in the Authorization header as "Bearer <token>".
    For this dummy server, a token is valid if it starts with "token_for_".
    """
    auth_header = request.headers.get("Authorization", None)
    if not auth_header or not auth_header.startswith("Bearer "):
        return jsonify({"status": "error", "message": "Missing or invalid token header"}), 401

    token = auth_header.split(" ")[1]
    if token.startswith("token_for_"):
        return jsonify({"status": "ok", "message": "Token is valid"}), 200
    else:
        return jsonify({"status": "error", "message": "Invalid token"}), 401

@app.route("/api/auth/refresh", methods=["POST"])
def refresh_token():
    """
    Refresh an expired token.
    Expects JSON with 'token'. For this prototype, simply append "_refreshed".
    """
    data = request.get_json()
    if not data:
        abort(400, description="Invalid JSON")
    old_token = data.get("token")
    if not old_token or not old_token.startswith("token_for_"):
        return jsonify({"status": "error", "message": "Invalid token provided"}), 401
    
    new_token = old_token + "_refreshed"
    return jsonify({"status": "ok", "token": new_token}), 200

@app.route("/api/auth/change-password", methods=["POST"])
def change_password():
    """
    Change a user's password.
    Expects JSON with 'username', 'old_password', and 'new_password'.
    """
    data = request.get_json()
    if not data:
        abort(400, description="Invalid JSON")
    username = data.get("username")
    old_password = data.get("old_password")
    new_password = data.get("new_password")
    if not username or not old_password or not new_password:
        abort(400, description="Missing required fields")
    
    user = users.get(username)
    if not user:
        return jsonify({"status": "error", "message": "User not found"}), 404
    if user["password"] != old_password:
        return jsonify({"status": "error", "message": "Old password is incorrect"}), 401
    # Update password (for prototype, storing as plain text)
    users[username]["password"] = new_password
    return jsonify({"status": "ok", "message": "Password changed successfully"}), 200

# Dummy data for classes (for /api/me/classes)
BIG_NOTES = {
    "math101": {
        "title": "Mathematics 101",
        "units": [
            {
                "unitId": "unit1",
                "title": "Introduction",
                "content": "<p>This unit covers basic math topics.</p>"
            },
            {
                "unitId": "unit2",
                "title": "Advanced Topics",
                "content": "<p>This unit covers advanced math topics.</p>"
            }
        ],
        "updatedAt": "2023-03-28T12:34:56Z",
    },
    "physics101": {
        "title": "Physics 101",
        "units": [
            {
                "unitId": "unit1",
                "title": "Introduction",
                "content": "<p>This unit covers basic physics topics.</p>"
            },
            {
                "unitId": "unit2",
                "title": "Quantum Physics",
                "content": "<p>This unit covers quantum physics.</p>"
            },
            {
                "unitId": "unit3",
                "title": "General Relativity",
                "content": "<p>This unit covers general relativity.</p>"
            }
        ],
        "updatedAt": "2023-03-28T12:34:56Z",
    },
    "chem101": {
        "title": "Chemistry 101",
        "units": [
            {
                "unitId": "unit1",
                "title": "Introduction",
                "content": "<p>This unit covers basic chemistry topics.</p>"
            }
        ],
        "updatedAt": "2023-03-28T12:34:56Z",
    },
}

@app.route("/api/me/classes", methods=["GET"])
def get_classes():
    """Return list of classes from dummy data."""
    classes = { key: {"id": key, "title": value["title"]} for key, value in BIG_NOTES.items() }
    if not classes:
        abort(404)
    return jsonify({"classes": classes}), 200

@app.route("/api/me/classes/<class_id>/bignote", methods=["GET"])
def get_class_bignote(class_id):
    """Return the full 'big note' for a specific class."""
    note = BIG_NOTES.get(class_id)
    if not note:
        return jsonify({"status": "error", "message": "Class not found"}), 404
    return jsonify({"status": "ok", "classId": class_id, "note": note}), 200

if __name__ == "__main__":
    # Running on all interfaces on port 3001
    app.run(host="0.0.0.0", port=3001)
