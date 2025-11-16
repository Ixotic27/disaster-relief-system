from flask import Flask, request, jsonify, send_from_directory
from flask_cors import CORS
import subprocess
import os
import json

app = Flask(__name__, static_folder='frontend', static_url_path='')
CORS(app)

# Paths
DATA_DIR = os.path.join('data')

# Correct: Pick .exe on Windows, 'resq' on Linux/Render
C_EXECUTABLE = os.path.join(
    'backend_c', 'build',
    'resq.exe' if os.name == 'nt' else 'resq'
)

# Ensure data directory exists
os.makedirs(DATA_DIR, exist_ok=True)

@app.route('/')
def serve_frontend():
    return send_from_directory('frontend', 'index.html')

@app.route('/<path:path>')
def serve_static(path):
    try:
        return send_from_directory('frontend', path)
    except:
        return send_from_directory('frontend', 'index.html')


@app.route('/generate', methods=['POST'])
def generate():
    try:
        data = request.json
        markers = data.get('markers', [])
        routes = data.get('routes', [])

        if not markers:
            return jsonify({'success': False, 'error': 'No markers provided'}), 400

        disasters = [m for m in markers if m['type'] == 'disaster']
        resources_list = [m for m in markers if m['type'] == 'resource']

        if not disasters:
            return jsonify({'success': False, 'error': 'No disaster zones provided'}), 400

        # regions.txt
        regions_path = os.path.join(DATA_DIR, 'regions.txt')
        with open(regions_path, 'w') as f:
            f.write('#ID,Name,Severity(optional),Population\n')
            for m in markers:
                sev = m.get('severity', 0) if m['type'] == 'disaster' else 0
                pop = m.get('population', 0)
                f.write(f"{m['id']},{m['name']},{sev},{pop}\n")

        # resources.txt (global)
        resources_path = os.path.join(DATA_DIR, 'resources.txt')
        with open(resources_path, 'w') as f:
            f.write('#ID,Name,Category,Quantity\n')
            f.write(f"R1,Rice,Food,{sum(m.get('resources',{}).get('Rice',0) for m in resources_list)}\n")
            f.write(f"R2,Water Bottles,Beverage,{sum(m.get('resources',{}).get('Water',0) for m in resources_list)}\n")
            f.write(f"R3,Blankets,Shelter,{sum(m.get('resources',{}).get('Blankets',0) for m in resources_list)}\n")
            f.write(f"R4,Medicines,Medical,{sum(m.get('resources',{}).get('Medicine',0) for m in resources_list)}\n")

        # region_resources.txt
        region_res_path = os.path.join(DATA_DIR, 'region_resources.txt')
        with open(region_res_path, 'w') as f:
            f.write('#region_index,region_id,resource_id,resource_name,quantity\n')
            idx = 0
            for m in markers:
                if m['type'] == 'resource':
                    for rid, label in [('R1','Rice'),('R2','Water Bottles'),
                                       ('R3','Blankets'),('R4','Medicines')]:
                        qty = m.get('resources', {}).get(label.split()[0], 0)
                        if qty > 0:
                            f.write(f"{idx},{m['id']},{rid},{label},{qty}\n")
                idx += 1

        # edges.txt (auto-generated)
        edges_path = os.path.join(DATA_DIR, 'edges.txt')
        with open(edges_path, 'w') as f:
            f.write('#From,To,Distance(km)\n')
            all_locs = disasters + resources_list
            written = set()

            # nearest neighbors
            for a in all_locs:
                d = []
                for b in all_locs:
                    if a['id'] != b['id']:
                        dist = int(haversine_distance(a['lat'], a['lon'], b['lat'], b['lon']))
                        d.append((a['name'], b['name'], dist))
                d.sort(key=lambda x: x[2])
                for x in d[:4]:
                    key = tuple(sorted([x[0], x[1]]))
                    if key not in written:
                        f.write(f"{x[0]},{x[1]},{x[2]}\n")
                        written.add(key)

            # guarantee resource → disaster
            for r in resources_list:
                for d in disasters:
                    key = tuple(sorted([r['name'], d['name']]))
                    if key not in written:
                        dist = int(haversine_distance(r['lat'], r['lon'], d['lat'], d['lon']))
                        f.write(f"{r['name']},{d['name']},{dist}\n")
                        written.add(key)

        # requests.txt
        requests_path = os.path.join(DATA_DIR, 'requests.txt')
        with open(requests_path, 'w') as f:
            f.write('#RequestID,RegionID,ResourceID,Quantity\n')
            q = 0
            for d in disasters:
                for rid in ['R1', 'R2', 'R3', 'R4']:
                    f.write(f"RQ_{q},{d['id']},{rid},{d.get('population',100000)}\n")
                    q += 1

        # check C executable exists
        if not os.path.exists(C_EXECUTABLE):
            return jsonify({
                'success': False,
                'error': f'C executable missing at {C_EXECUTABLE}',
                'details': 'Render cannot run Windows .exe on Linux. build.sh builds "resq".'
            }), 500

        # RUN C executable
        exe_path = os.path.abspath(C_EXECUTABLE)
        result = subprocess.run(
            [exe_path], cwd=os.path.abspath('backend_c'),
            capture_output=True, text=True, timeout=30
        )

        if result.returncode != 0:
            return jsonify({'success': False, 'error': 'C program failed', 'details': result.stderr}), 500

        # read report
        rep = os.path.join(DATA_DIR, 'report.txt')
        if not os.path.exists(rep):
            return jsonify({'success': False, 'error': 'report.txt missing'}), 500
        
        return jsonify({'success': True, 'report': open(rep).read(), 'stdout': result.stdout})

    except subprocess.TimeoutExpired:
        return jsonify({'success': False, 'error': 'C execution timeout'}), 500
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 500


@app.route('/report', methods=['GET'])
def report():
    try:
        path = os.path.join(DATA_DIR, 'report.txt')
        if not os.path.exists(path):
            return jsonify({'success': False, 'error': 'Report not found'}), 404
        return jsonify({'success': True, 'report': open(path).read()})
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)})


def haversine_distance(lat1, lon1, lat2, lon2):
    from math import radians, sin, cos, sqrt, atan2
    R = 6371
    lat1, lon1, lat2, lon2 = map(radians, [lat1, lon1, lat2, lon2])
    dlat = lat2 - lat1
    dlon = lon2 - lon1
    a = sin(dlat/2)**2 + cos(lat1)*cos(lat2)*(sin(dlon/2)**2)
    return R * (2 * atan2(sqrt(a), sqrt(1-a)))

# -------------------- FIXED RENDER STARTER --------------------
if __name__ == '__main__':
    port = int(os.environ.get('PORT', 5000))

    print("=" * 60)
    print("Disaster Relief Allocation System - Flask Backend")
    print("=" * 60)
    print(f"Data directory: {os.path.abspath(DATA_DIR)}")
    print(f"C executable: {os.path.abspath(C_EXECUTABLE)}")
    print(f"Starting server on PORT={port}")
    print("=" * 60)

    app.run(host='0.0.0.0', port=port, debug=False)
