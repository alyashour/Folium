"""
This is used to check if the webapp server indicator (bottom right) works as intended.
Run, set an env var as `PUBLIC_LEDGER_PING_URL=http://10.0.0.188:3001/ping` and check the indicator - it should be green.
"""

from flask import Flask, jsonify
from flask_cors import CORS

app = Flask(__name__)
CORS(app)  # This will allow requests from any origin

@app.route('/ping', methods=['GET'])
def ping():
    return jsonify(status='ok', message='pong'), 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=3001)
