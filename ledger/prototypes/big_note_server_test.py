"""
Prototype Python Server for Folium

This server implements basic authentication endpoints:
  - /ping: Health check endpoint.
  - /api/auth/register: Endpoint to register a new user.
  - /api/auth/login: Endpoint to login an existing user.
  - /api/me/classes: Endpoint to return available class information.

It also seeds a default user ("admin" with password "password") if not already present.
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
        # Default password is "password"; in production, use secure password hashing!
        users["admin"] = {"password": "password", "role": "admin"}
        print("Default user 'admin' seeded.")

seed_default_user()

@app.route("/ping", methods=["GET"])
def ping():
    """Health check endpoint."""
    return jsonify(status="ok", message="pong"), 200

@app.route("/api/auth/register", methods=["POST"])
def register():
    """
    Register a new user.
    Expects JSON payload with 'username' and 'password'.
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
    """
    Login an existing user.
    Expects JSON payload with 'username' and 'password'.
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
        # In a real server, you'd return a JWT or similar token
        token = f"token_for_{username}"
        return jsonify({"status": "ok", "token": token, "role": user["role"]}), 200
    else:
        return jsonify({"status": "error", "message": "Invalid credentials"}), 401

# Dummy data for classes (similar to your existing BIG_NOTES)
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

if __name__ == "__main__":
    # Running on all interfaces on port 3001 (adjust if needed)
    app.run(host="0.0.0.0", port=3001)
