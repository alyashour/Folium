"""
This allows me to test if the frontend can pull notes as JSON from the service.
"""
from flask import Flask, jsonify, abort
from flask_cors import CORS

app = Flask(__name__)
CORS(app)  # Allow requests from different origins for testing
 
# Dummy data for demonstration purposes
BIG_NOTES = {
    "math101": {
        "title": "Mathematics 101 Big Note",
        "units": [
            {
                "unitId": "unit1",
                "title": "Introduction",
                "content": "<p>This unit covers the basics of math, including numbers, operations, and basic algebra.</p>"
            },
            {
                "unitId": "unit2",
                "title": "Advanced Topics",
                "content": "<p>This unit dives into advanced concepts like calculus and statistics.</p>"
            }
        ],
        "updatedAt": "2023-03-28T12:34:56Z"
    }
}

@app.route('/ping', methods=['GET'])
def ping():
    return jsonify(status='ok', message='pong'), 200

@app.route('/api/classes/<class_id>/bignote', methods=['GET'])
def get_big_note(class_id):
    big_note = BIG_NOTES.get(class_id)
    if not big_note:
        abort(404)
    return jsonify({
        "classId": class_id,
        "bigNote": big_note
    })

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=3001)
