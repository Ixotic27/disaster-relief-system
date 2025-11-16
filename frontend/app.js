// Global variables
let map, markers = [], polylines = [], markerIdCounter = 0, disasterIcon, resourceIcon;

// Initialize on page load
window.addEventListener('load', () => {
    initMap();
    attachEventListeners();
});

function initMap() {
    // Initialize map centered on Uttarakhand, India
    map = L.map('map').setView([30.0668, 79.0193], 8);
    
    // Add OpenStreetMap tiles
    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
        attribution: '© OpenStreetMap contributors',
        maxZoom: 19
    }).addTo(map);

    // Create custom icons
    disasterIcon = L.icon({
        iconUrl: 'data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iMzAiIGhlaWdodD0iMzAiIHZpZXdCb3g9IjAgMCAzMCAzMCIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj48Y2lyY2xlIGN4PSIxNSIgY3k9IjE1IiByPSIxMiIgZmlsbD0iI2U3NGMzYyIgc3Ryb2tlPSIjYzAzYTJiIiBzdHJva2Utd2lkdGg9IjIiLz48L3N2Zz4=',
        iconSize: [30, 30],
        iconAnchor: [15, 15],
        popupAnchor: [0, -15]
    });

    resourceIcon = L.icon({
        iconUrl: 'data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iMzAiIGhlaWdodD0iMzAiIHZpZXdCb3g9IjAgMCAzMCAzMCIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj48Y2lyY2xlIGN4PSIxNSIgY3k9IjE1IiByPSIxMiIgZmlsbD0iIzI3YWU2MCIgc3Ryb2tlPSIjMWU4NDRiIiBzdHJva2Utd2lkdGg9IjIiLz48L3N2Zz4=',
        iconSize: [30, 30],
        iconAnchor: [15, 15],
        popupAnchor: [0, -15]
    });

    // Map click handler
    map.on('click', function(e) {
        const name = prompt('Enter location name:');
        if (!name) return;
        const type = confirm('Is this a disaster zone?\n\nOK = Disaster Zone\nCancel = Resource Center') ? 'disaster' : 'resource';
        createMarker(e.latlng.lat, e.latlng.lng, name, type);
    });
}

function attachEventListeners() {
    const listeners = {
        'btn-mark-disaster': () => addMarkerFromSearch('disaster'),
        'btn-mark-resource': () => addMarkerFromSearch('resource'),
        'btn-sample-data': loadSampleData,
        'btn-generate-run': generateAndRun
    };
    Object.entries(listeners).forEach(([id, fn]) => document.getElementById(id).addEventListener('click', fn));
}

// Geocode location using Nominatim
async function geocodeLocation(locationName) {
    const url = `https://nominatim.openstreetmap.org/search?format=json&q=${encodeURIComponent(locationName)}&limit=1`;
    try {
        const response = await fetch(url);
        const data = await response.json();
        if (data.length > 0) {
            return {
                lat: parseFloat(data[0].lat),
                lon: parseFloat(data[0].lon),
                name: data[0].display_name
            };
        }
        return null;
    } catch (error) {
        console.error('Geocoding error:', error);
        return null;
    }
}

// Add marker from search input
async function addMarkerFromSearch(type) {
    const input = document.getElementById('location-input').value.trim();
    if (!input) {
        showStatus('Please enter a location name', 'error');
        return;
    }

    showStatus('Searching for location...', 'info');
    const result = await geocodeLocation(input);
    
    if (!result) {
        showStatus('Location not found. Try a different name or click on the map.', 'error');
        return;
    }

    createMarker(result.lat, result.lon, input, type);
    document.getElementById('location-input').value = '';
    showStatus(`Added ${type} marker: ${input}`, 'success');
}

// Create marker on map
function createMarker(lat, lon, name, type) {
    const id = `RG${markerIdCounter + 1}`;
    markerIdCounter++;
    
    const icon = type === 'disaster' ? disasterIcon : resourceIcon;
    const marker = L.marker([lat, lon], { icon }).addTo(map);
    marker.bindPopup(`<b>${name}</b><br>Type: ${type.toUpperCase()}`);
    
    const markerData = {
        id,
        name,
        lat,
        lon,
        type,
        severity: 5,
        population: type === 'disaster' ? 100000 : 0,
        resources: type === 'resource' ? {
            Rice: 1000,
            Water: 2000,
            Blankets: 500,
            Medicine: 300
        } : {}
    };
    
    markers.push(markerData);
    marker.markerData = markerData;
    
    updateMarkerList();
    map.setView([lat, lon], 10);
}

// Update marker list in sidebar
function updateMarkerList() {
    const list = document.getElementById('marker-list');
    list.innerHTML = '';
    
    if (markers.length === 0) {
        list.innerHTML = '<li class="empty-state">No locations added yet</li>';
        document.getElementById('marker-count').textContent = '0';
        return;
    }
    
    markers.forEach(m => {
        const li = document.createElement('li');
        li.className = `marker-item ${m.type}`;
        
        let controlsHTML = '';
        if (m.type === 'disaster') {
            controlsHTML = `
                <label>Severity (1-10): <span class="severity-display" id="sev-${m.id}">${m.severity}</span></label>
                <input type="range" min="1" max="10" value="${m.severity}" 
                       onchange="updateSeverity('${m.id}', this.value)">
                <label>Population:</label>
                <input type="number" value="${m.population}" 
                       onchange="updatePopulation('${m.id}', this.value)">
            `;
        } else {
            controlsHTML = `
                <label>Available Resources:</label>
                <div class="resource-inputs">
                    <div>
                        <label style="font-size: 11px; color: #888;">🌾 Rice</label>
                        <input type="number" value="${m.resources.Rice || 0}" 
                               onchange="updateResource('${m.id}', 'Rice', this.value)">
                    </div>
                    <div>
                        <label style="font-size: 11px; color: #888;">💧 Water</label>
                        <input type="number" value="${m.resources.Water || 0}" 
                               onchange="updateResource('${m.id}', 'Water', this.value)">
                    </div>
                    <div>
                        <label style="font-size: 11px; color: #888;">🛏️ Blankets</label>
                        <input type="number" value="${m.resources.Blankets || 0}" 
                               onchange="updateResource('${m.id}', 'Blankets', this.value)">
                    </div>
                    <div>
                        <label style="font-size: 11px; color: #888;">💊 Medicine</label>
                        <input type="number" value="${m.resources.Medicine || 0}" 
                               onchange="updateResource('${m.id}', 'Medicine', this.value)">
                    </div>
                </div>
            `;
        }
        
        li.innerHTML = `
            <h3>${m.name} (${m.type})</h3>
            <div class="marker-controls">
                ${controlsHTML}
                <button class="btn-secondary" style="margin-top: 8px;" onclick="removeMarker('${m.id}')">Remove</button>
            </div>
        `;
        list.appendChild(li);
    });
    
    document.getElementById('marker-count').textContent = markers.length;
}

function updateSeverity(id, val) {
    const marker = markers.find(m => m.id === id);
    if (marker) {
        marker.severity = parseInt(val);
        document.getElementById(`sev-${id}`).textContent = val;
    }
}

function updatePopulation(id, val) {
    const marker = markers.find(m => m.id === id);
    if (marker) marker.population = parseInt(val) || 0;
}

function updateResource(id, resourceName, val) {
    const marker = markers.find(m => m.id === id);
    if (marker) marker.resources[resourceName] = parseInt(val) || 0;
}

function removeMarker(id) {
    markers = markers.filter(m => m.id !== id);
    map.eachLayer(layer => layer instanceof L.Marker && layer.markerData?.id === id && map.removeLayer(layer));
    updateMarkerList();
}

// Haversine distance formula (returns km)
const haversineDistance = (lat1, lon1, lat2, lon2) => {
    const R = 6371, toRad = deg => deg * Math.PI / 180;
    const dLat = toRad(lat2 - lat1), dLon = toRad(lon2 - lon1);
    const a = Math.sin(dLat/2) ** 2 + Math.cos(toRad(lat1)) * Math.cos(toRad(lat2)) * Math.sin(dLon/2) ** 2;
    return R * 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1-a));
};

// Generate and run allocation
async function generateAndRun() {
    if (markers.length === 0) {
        showStatus('Please add at least one location', 'error');
        return;
    }

    const disasters = markers.filter(m => m.type === 'disaster');
    const resources = markers.filter(m => m.type === 'resource');

    if (disasters.length === 0) {
        showStatus('Please add at least one disaster zone', 'error');
        return;
    }

    if (resources.length === 0) {
        showStatus('Please add at least one resource center', 'error');
        return;
    }

    showStatus('Generating files and running allocation...', 'info');

    const payload = {
        markers: markers,
        routes: []
    };

    try {
        const response = await fetch('/generate', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(payload)
        });

        const result = await response.json();
        
        if (result.success) {
            showStatus('Allocation completed successfully!', 'success');
            displayResults(result);
        } else {
            showStatus(`Error: ${result.error}`, 'error');
            if (result.details) {
                document.getElementById('report-output').textContent = result.details;
            }
        }
    } catch (error) {
        showStatus(`Request failed: ${error.message}`, 'error');
        console.error('Error:', error);
    }
}


// Display results on map and in sidebar
async function displayResults(result) {
    // Clear existing polylines
    polylines.forEach(p => map.removeLayer(p));
    polylines = [];

    // Parse CSV report
    const lines = result.report.split('\n');
    const allocations = [];
    
    for (let i = 1; i < lines.length; i++) {
        if (lines[i].trim()) {
            const parts = lines[i].split(',');
            if (parts.length >= 7) {
                allocations.push({
                    reqId: parts[0],
                    regionId: parts[1],
                    resourceId: parts[2],
                    allocated: parseInt(parts[3]),
                    route: parts[4],
                    cost: parseInt(parts[5]),
                    status: parts[6].trim(),
                    enhancedRoute: null // Will be filled with OSRM data
                });
            }
        }
    }

    // Map resource IDs to names
    const resourceNames = {
        'R1': 'Rice',
        'R2': 'Water Bottles',
        'R3': 'Blankets',
        'R4': 'Medicines'
    };

    // Get region name from ID
    function getRegionName(regionId) {
        const marker = markers.find(m => m.id === regionId);
        return marker ? marker.name : regionId;
    }

    // Draw routes on map and enhance route information with OSRM data
    const drawnRoutes = new Set();
    
    for (const a of allocations) {
        if (a.route && a.route !== 'NO_PATH' && a.allocated > 0) {
            const routeParts = a.route.split(' -> ');
            const routeKey = routeParts.join('->');
            
            if (drawnRoutes.has(routeKey)) continue;
            drawnRoutes.add(routeKey);
            
            const coords = [];
            
            routeParts.forEach(name => {
                const marker = markers.find(m => m.name.toLowerCase() === name.toLowerCase());
                if (marker) {
                    coords.push([marker.lat, marker.lon]);
                }
            });

            if (coords.length >= 2) {
                const color = a.status.includes('FULFILLED') ? '#27ae60' : '#f39c12';
                const resourceName = resourceNames[a.resourceId] || a.resourceId;
                
                // Get enhanced route with intermediate cities
                const enhancedRoute = await drawRoadRouteAndGetWaypoints(coords, color, resourceName, a.allocated, a.status, a.cost, routeParts);
                a.enhancedRoute = enhancedRoute;
                
                // Add yellow waypoint markers for intermediate cities
                for (let i = 1; i < coords.length - 1; i++) {
                    const waypointIcon = L.divIcon({
                        className: 'waypoint-marker',
                        html: '<div style="background: #fbbf24; border: 3px solid white; width: 14px; height: 14px; border-radius: 50%; box-shadow: 0 2px 6px rgba(0,0,0,0.4);"></div>',
                        iconSize: [14, 14],
                        iconAnchor: [7, 7]
                    });
                    
                    const waypointMarker = L.marker(coords[i], { icon: waypointIcon }).addTo(map);
                    waypointMarker.bindPopup(`<b>📍 Via: ${routeParts[i]}</b>`);
                    polylines.push(waypointMarker);
                }
            }
        }
    }

    // Display in console format with enhanced routes
    let consoleHTML = '<div class="console-output">';
    consoleHTML += '<div class="console-line console-header">📊 ALLOCATION REPORT GENERATED</div>';
    consoleHTML += `<div class="console-line"><span class="console-label">Total Allocations:</span> <span class="console-value">${allocations.length}</span></div>`;
    consoleHTML += '<div style="height: 15px;"></div>';
    
    // Group by region
    const byRegion = {};
    allocations.forEach(a => {
        if (!byRegion[a.regionId]) {
            byRegion[a.regionId] = [];
        }
        byRegion[a.regionId].push(a);
    });

    Object.keys(byRegion).forEach(regionId => {
        const regionName = getRegionName(regionId);
        consoleHTML += `<div class="console-line console-header">🔴 DISASTER ZONE: ${regionName.toUpperCase()}</div>`;
        
        byRegion[regionId].forEach(a => {
            const resourceName = resourceNames[a.resourceId] || a.resourceId;
            const statusClass = a.status.includes('FULFILLED') ? 'console-fulfilled' : 
                               a.status.includes('PARTIAL') ? 'console-partial' : 'console-failed';
            
            consoleHTML += `<div class="console-line">`;
            consoleHTML += `  <span class="console-label">Resource:</span> <span class="console-value">${resourceName}</span><br>`;
            consoleHTML += `  <span class="console-label">Allocated:</span> <span class="console-value">${a.allocated.toLocaleString()} units</span><br>`;
            consoleHTML += `  <span class="console-label">Status:</span> <span class="${statusClass}">${a.status}</span><br>`;
            
            if (a.route && a.route !== 'NO_PATH') {
                // Use enhanced route if available, otherwise use basic route
                const displayRoute = a.enhancedRoute || a.route.split(' -> ').map(name => 
                    name.charAt(0).toUpperCase() + name.slice(1)
                ).join(' → ');
                
                consoleHTML += `  <span class="console-label">Route:</span> <span class="console-route">${displayRoute}</span><br>`;
                consoleHTML += `  <span class="console-label">Distance:</span> <span class="console-value">${a.cost} km</span>`;
            } else {
                consoleHTML += `  <span class="console-label">Route:</span> <span class="console-failed">NO PATH AVAILABLE</span>`;
            }
            consoleHTML += `</div>`;
        });
        
        consoleHTML += '<div style="height: 10px;"></div>';
    });
    
    consoleHTML += '</div>';
    document.getElementById('report-output').innerHTML = consoleHTML;
}

// Function to draw road-following route using OSRM and return waypoint cities
async function drawRoadRouteAndGetWaypoints(coords, color, resourceName, allocated, status, distance, routeParts) {
    try {
        // Build OSRM API URL for route with steps
        const coordsStr = coords.map(c => `${c[1]},${c[0]}`).join(';');
        const osrmUrl = `https://router.project-osrm.org/route/v1/driving/${coordsStr}?overview=full&geometries=geojson&steps=true`;
        
        const response = await fetch(osrmUrl);
        const data = await response.json();
        
        if (data.code === 'Ok' && data.routes && data.routes.length > 0) {
            const route = data.routes[0];
            const routeCoords = route.geometry.coordinates.map(c => [c[1], c[0]]);
            
            // Extract city names from steps if available
            let waypointCities = [routeParts[0]]; // Start city
            
            // Check which of our marked locations fall along the route
            for (let i = 1; i < routeParts.length - 1; i++) {
                waypointCities.push(routeParts[i]);
            }
            
            waypointCities.push(routeParts[routeParts.length - 1]); // End city
            
            // Draw the road-following polyline
            const polyline = L.polyline(routeCoords, {
                color: color,
                weight: 5,
                opacity: 0.8
            }).addTo(map);
            
            const enhancedRoute = waypointCities.map(c => 
                c.charAt(0).toUpperCase() + c.slice(1)
            ).join(' → ');
            
            polyline.bindPopup(`
                <b>${resourceName}</b><br>
                Quantity: ${allocated.toLocaleString()} units<br>
                Status: <b>${status}</b><br>
                Distance: ${distance} km<br>
                Route: ${enhancedRoute}
            `);
            
            polylines.push(polyline);
            
            return enhancedRoute;
        } else {
            // Fallback to straight line
            const polyline = L.polyline(coords, {
                color: color,
                weight: 4,
                opacity: 0.7,
                dashArray: '5, 10'
            }).addTo(map);
            
            const fallbackRoute = routeParts.map(c => 
                c.charAt(0).toUpperCase() + c.slice(1)
            ).join(' → ');
            
            polyline.bindPopup(`
                <b>${resourceName}</b><br>
                Quantity: ${allocated.toLocaleString()} units<br>
                Status: <b>${status}</b><br>
                Route: ${fallbackRoute}<br>
                <small>(Direct route - detailed road data unavailable)</small>
            `);
            
            polylines.push(polyline);
            
            return fallbackRoute;
        }
    } catch (error) {
        console.error('OSRM routing error:', error);
        // Fallback
        const polyline = L.polyline(coords, {
            color: color,
            weight: 4,
            opacity: 0.6
        }).addTo(map);
        polylines.push(polyline);
        
        return routeParts.map(c => c.charAt(0).toUpperCase() + c.slice(1)).join(' → ');
    }
}

// Load sample data for testing
function loadSampleData() {
    markers = [];
    markerIdCounter = 0;
    map.eachLayer(layer => layer instanceof L.Marker && map.removeLayer(layer));
    polylines.forEach(p => map.removeLayer(p));
    polylines = [];

    const sampleData = [
        { lat: 30.3165, lon: 78.0322, name: 'Dehradun', type: 'disaster', pop: 5000, sev: 8 },
        { lat: 29.9457, lon: 78.1642, name: 'Haridwar', type: 'resource', res: { Rice: 2000, Water: 500, Blankets: 0, Medicine: 0 } },
        { lat: 30.3909, lon: 78.4343, name: 'Tehri', type: 'resource', res: { Rice: 0, Water: 1500, Blankets: 1000, Medicine: 0 } },
        { lat: 30.1534, lon: 78.9712, name: 'Pauri', type: 'resource', res: { Rice: 1000, Water: 0, Blankets: 500, Medicine: 800 } }
    ];

    sampleData.forEach(d => {
        createMarker(d.lat, d.lon, d.name, d.type);
        const m = markers[markers.length - 1];
        if (d.type === 'disaster') {
            m.population = d.pop;
            m.severity = d.sev;
        } else {
            m.resources = d.res;
        }
    });
    
    updateMarkerList();
    showStatus('Sample data loaded - Multiple resource centers with different supplies', 'success');
}

function showStatus(message, type) {
    const status = document.getElementById('status');
    status.className = `status ${type}`;
    status.textContent = message;
    if (type !== 'info') setTimeout(() => status.className = 'status', 5000);
}