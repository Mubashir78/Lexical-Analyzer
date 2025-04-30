from flask_cors import CORS
import subprocess
import shlex
from flask import Flask, send_from_directory, request, jsonify
import os

# Initialize Flask app ONCE with both configurations
app = Flask(__name__, static_folder=os.path.dirname(os.path.abspath(__file__)))
CORS(app)  # Enable CORS for the app

@app.route('/')
def serve_index():
    return send_from_directory(app.static_folder, 'index.html')

@app.route('/analyze', methods=['POST'])
def analyze():
    expr = request.json.get('expression', '')
    if not expr:
        return jsonify({'error': 'Empty expression'}), 400
    
    try:
        result = subprocess.run(
            ['./math', shlex.quote(expr)],
            capture_output=True,
            text=True
        )
        return jsonify({
            'output': result.stdout,
            'error': result.stderr
        })
    except Exception as e:
        return jsonify({'error': str(e)}), 500

if __name__ == '__main__':
    app.run(port=8000)