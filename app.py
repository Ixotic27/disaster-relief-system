from flask import Flask, request, jsonify, send_from_directory
from flask_cors import CORS
import subprocess
import os
import json

app = Flask(__name__, static_folder='.', static_url_path='')
CORS(app)

# Paths
DATA_DIR = os.path.join('data')
C_EXECUTABLE = os.path.join('backend_c', 'build', 'resq.exe')  # Windows executable

# Ensure data directory exists
os.makedirs(DATA_DIR, exist_ok=True)


@app.route('/')
def serve_frontend():
    return send_from_directory('.', 'index.html')


@app.route('/<path:filename>')
def serve_static(filename):
    return send_from_directory('.', filename)


@app.route('/generate', methods=['POST'])
def generate():
    try:
        data = request.json
        markers = data.get('markers', [])
        routes = data.get('routes', [])

        if not markers:
            return jsonify({'success': False, 'error': 'No markers provided'}), 400

        # Separate disasters and resources
        disasters = [m for m in markers if m['type'] == 'disaster']
        resources_list = [m for m in markers if m['type'] == 'resource']

        if not disasters:
            return jsonify({'success': False, 'error': 'No disaster zones provided'}), 400

        # Generate regions.txt
        regions_path = os.path.join(DATA_DIR, 'regions.txt')
        with open(regions_path, 'w') as f:
            f.write('#ID,Name,Severity(optional),Population\n')
            for marker in markers:
                severity = marker.get('severity', 0) if marker['type'] == 'disaster' else 0
                population = marker.get('population', 0)
                f.write(f"{marker['id']},{marker['name']},{severity},{population}\n")

        print(f"✓ Generated {regions_path}")

        # Generate resources.txt - KEEP PER-REGION tracking (don't aggregate)
        resources_path = os.path.join(DATA_DIR, 'resources.txt')
        with open(resources_path, 'w') as f:
            f.write('#ID,Name,Category,Quantity\n')
            
            # Calculate TOTAL available across all resource centers
            total_rice = sum(m.get('resources', {}).get('Rice', 0) for m in resources_list)
            total_water = sum(m.get('resources', {}).get('Water', 0) for m in resources_list)
            total_blankets = sum(m.get('resources', {}).get('Blankets', 0) for m in resources_list)
            total_medicine = sum(m.get('resources', {}).get('Medicine', 0) for m in resources_list)
            
            # Write global totals (C program allocates from global pool)
            f.write(f"R1,Rice,Food,{total_rice}\n")
            f.write(f"R2,Water Bottles,Beverage,{total_water}\n")
            f.write(f"R3,Blankets,Shelter,{total_blankets}\n")
            f.write(f"R4,Medicines,Medical,{total_medicine}\n")
        
        print(f"✓ Generated {resources_path}")
        print(f"  Total resources: Rice={total_rice}, Water={total_water}, Blankets={total_blankets}, Medicine={total_medicine}")
        
        # Also save per-region resource mapping for reference
        region_res_path = os.path.join(DATA_DIR, 'region_resources_map.txt')
        with open(region_res_path, 'w') as f:
            f.write('#RegionID,RegionName,Rice,Water,Blankets,Medicine\n')
            for res_center in resources_list:
                f.write(f"{res_center['id']},{res_center['name']},"
                       f"{res_center.get('resources', {}).get('Rice', 0)},"
                       f"{res_center.get('resources', {}).get('Water', 0)},"
                       f"{res_center.get('resources', {}).get('Blankets', 0)},"
                       f"{res_center.get('resources', {}).get('Medicine', 0)}\n")

        print(f"✓ Generated {resources_path}")

        # Generate edges.txt - Create intelligent network topology
        edges_path = os.path.join(DATA_DIR, 'edges.txt')
        with open(edges_path, 'w') as f:
            f.write('#From,To,Distance(km)\n')
            if routes and len(routes) > 0:
                # User manually created routes - use them
                for route in routes:
                    f.write(f"{route['from']},{route['to']},{route['distance']}\n")
            else:
                # Auto-generate intelligent network topology
                # Connect each location to its 2-3 nearest neighbors (not full mesh)
                # This creates multi-hop paths
                all_locations = disasters + resources_list
                
                for loc in all_locations:
                    # Calculate distances to all other locations
                    distances = []
                    for other in all_locations:
                        if loc['id'] != other['id']:
                            dist = haversine_distance(
                                loc['lat'], loc['lon'],
                                other['lat'], other['lon']
                            )
                            distances.append({
                                'to': other['name'],
                                'distance': int(dist)
                            })
                    
                    # Sort by distance and connect to 2-3 nearest neighbors
                    distances.sort(key=lambda x: x['distance'])
                    max_connections = min(3, len(distances))  # Connect to 2-3 nearest
                    
                    for i in range(max_connections):
                        f.write(f"{loc['name']},{distances[i]['to']},{distances[i]['distance']}\n")

        print(f"✓ Generated {edges_path}")

        # Generate requests.txt (optional - C program might generate this internally)
        requests_path = os.path.join(DATA_DIR, 'requests.txt')
        with open(requests_path, 'w') as f:
            f.write('#RequestID,RegionID,ResourceID,Quantity\n')
            req_id = 0
            for disaster in disasters:
                for res_id in ['R1', 'R2', 'R3', 'R4']:
                    f.write(f"RQ_{req_id},{disaster['id']},{res_id},{disaster.get('population', 100000)}\n")
                    req_id += 1

        print(f"✓ Generated {requests_path}")

        # Run C executable
        if not os.path.exists(C_EXECUTABLE):
            return jsonify({
                'success': False,
                'error': f'C executable not found at: {C_EXECUTABLE}',
                'details': 'Please compile: cd backend_c && gcc src/*.c -Iinclude -o build/resq.exe'
            }), 500

        print(f"Running C executable: {C_EXECUTABLE}")

        # Run the C program from backend_c directory
        result = subprocess.run(
            [os.path.abspath(C_EXECUTABLE)],
            cwd=os.path.abspath('backend_c'),
            capture_output=True,
            text=True,
            timeout=30
        )

        print(f"C program stdout:\n{result.stdout}")
        if result.stderr:
            print(f"C program stderr:\n{result.stderr}")

        if result.returncode != 0:
            return jsonify({
                'success': False,
                'error': f'C program failed with return code {result.returncode}',
                'details': result.stderr or result.stdout
            }), 500

        # Read report.txt
        report_path = os.path.join(DATA_DIR, 'report.txt')
        if not os.path.exists(report_path):
            return jsonify({
                'success': False,
                'error': 'Report file not generated',
                'details': result.stdout
            }), 500

        with open(report_path, 'r') as f:
            report_content = f.read()

        return jsonify({
            'success': True,
            'report': report_content,
            'stdout': result.stdout
        })

    except subprocess.TimeoutExpired:
        return jsonify({
            'success': False,
            'error': 'C program execution timed out (30s limit)'
        }), 500
    except Exception as e:
        return jsonify({
            'success': False,
            'error': str(e),
            'details': str(type(e).__name__)
        }), 500


@app.route('/report', methods=['GET'])
def get_report():
    try:
        report_path = os.path.join(DATA_DIR, 'report.txt')
        if not os.path.exists(report_path):
            return jsonify({'success': False, 'error': 'Report not found'}), 404

        with open(report_path, 'r') as f:
            report_content = f.read()

        return jsonify({'success': True, 'report': report_content})
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 500


def haversine_distance(lat1, lon1, lat2, lon2):
    """Calculate distance between two points using Haversine formula (returns km)"""
    from math import radians, sin, cos, sqrt, atan2
    
    R = 6371  # Earth radius in km
    
    lat1, lon1, lat2, lon2 = map(radians, [lat1, lon1, lat2, lon2])
    dlat = lat2 - lat1
    dlon = lon2 - lon1
    
    a = sin(dlat/2)**2 + cos(lat1) * cos(lat2) * sin(dlon/2)**2
    c = 2 * atan2(sqrt(a), sqrt(1-a))
    
    return R * c


if __name__ == '__main__':
    print("=" * 60)
    print("Disaster Relief Allocation System - Flask Backend")
    print("=" * 60)
    print(f"Data directory: {os.path.abspath(DATA_DIR)}")
    print(f"C executable: {os.path.abspath(C_EXECUTABLE)}")
    print(f"Frontend will be served at: http://localhost:5000")
    print("=" * 60)
    app.run(debug=False, port=5000, use_reloader=False)  # Disabled reloader to prevent crash