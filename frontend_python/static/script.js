// Region data with coordinates for Uttarakhand
const regions = [
    { id: 1, name: 'Dehradun', lat: 30.3165, lng: 78.0322, color: '#3b82f6' },
    { id: 2, name: 'Haridwar', lat: 29.9457, lng: 78.1642, color: '#10b981' },
    { id: 3, name: 'Tehri', lat: 30.3909, lng: 78.4800, color: '#f59e0b' },
    { id: 4, name: 'Pauri', lat: 30.1535, lng: 78.7757, color: '#ef4444' },
    { id: 5, name: 'Chamoli', lat: 30.4000, lng: 79.3300, color: '#8b5cf6' },
    { id: 6, name: 'Nainital', lat: 29.3803, lng: 79.4636, color: '#ec4899' },
    { id: 7, name: 'Almora', lat: 29.5971, lng: 79.6591, color: '#06b6d4' }
];

let map;
let markers = {};
let regionStates = {};

// Initialize the application
document.addEventListener('DOMContentLoaded', () => {
    initializeMap();
    renderRegions();
    attachEventListeners();
});

// Initialize Leaflet map
function initializeMap() {
    map = L.map('map').setView([30.0668, 79.0193], 8);
    
    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
        attribution: '© OpenStreetMap contributors'
    }).addTo(map);

    // Add markers for each region
    regions.forEach(region => {
        const marker = L.circleMarker([region.lat, region.lng], {
            radius: 8,
            fillColor: '#cbd5e1',
            color: '#64748b',
            weight: 2,
            opacity: 1,
            fillOpacity: 0.6
        }).addTo(map);
        
        marker.bindPopup(`<b>${region.name}</b><br>Click to toggle`);
        marker.on('click', () => toggleRegion(region.id));
        markers[region.id] = marker;
    });
}

// Render region cards
function renderRegions() {
    const regionList = document.getElementById('region-list');
    
    regions.forEach(region => {
        regionStates[region.id] = { selected: false, severity: 5 };
        
        const card = document.createElement('div');
        card.className = 'region-card bg-gray-50 rounded-lg p-4 border-2 border-gray-200';
        card.id = `region-${region.id}`;
        
        card.innerHTML = `
            <div class="flex items-center justify-between mb-3">
                <div class="flex items-center space-x-3">
                    <label class="relative inline-flex items-center cursor-pointer">
                        <input type="checkbox" class="sr-only peer region-checkbox" data-region-id="${region.id}">
                        <div class="w-11 h-6 bg-gray-300 peer-focus:outline-none peer-focus:ring-4 peer-focus:ring-blue-300 rounded-full peer peer-checked:after:translate-x-full peer-checked:after:border-white after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:border-gray-300 after:border after:rounded-full after:h-5 after:w-5 after:transition-all peer-checked:bg-blue-600"></div>
                    </label>
                    <span class="font-semibold text-gray-700">${region.name}</span>
                </div>
                <span class="severity-badge text-sm font-bold px-2 py-1 rounded" style="background-color: ${getSeverityColor(5)}; color: white;">
                    Level <span class="severity-value">${5}</span>
                </span>
            </div>
            <div class="flex items-center space-x-3">
                <span class="text-xs text-gray-500 w-16">Severity:</span>
                <input type="range" min="1" max="10" value="5" 
                    class="flex-1 h-2 rounded-lg appearance-none cursor-pointer severity-slider"
                    data-region-id="${region.id}" disabled>
                <span class="text-sm font-semibold text-gray-700 w-8 text-right severity-display">5</span>
            </div>
            <div class="mt-2 bg-gray-200 rounded-full h-2 overflow-hidden">
                <div class="progress-bar h-full transition-all duration-300" 
                    style="width: 50%; background-color: ${getSeverityColor(5)}"></div>
            </div>
        `;
        
        regionList.appendChild(card);
    });
}

// Attach event listeners
function attachEventListeners() {
    // Region checkboxes
    document.querySelectorAll('.region-checkbox').forEach(checkbox => {
        checkbox.addEventListener('change', (e) => {
            const regionId = parseInt(e.target.dataset.regionId);
            toggleRegion(regionId);
        });
    });

    // Severity sliders
    document.querySelectorAll('.severity-slider').forEach(slider => {
        slider.addEventListener('input', (e) => {
            const regionId = parseInt(e.target.dataset.regionId);
            const severity = parseInt(e.target.value);
            updateSeverity(regionId, severity);
        });
    });

    // Control buttons
    document.getElementById('select-all-btn').addEventListener('click', selectAll);
    document.getElementById('clear-all-btn').addEventListener('click', clearAll);
    document.getElementById('run-allocation-btn').addEventListener('click', runAllocation);
}

// Toggle region selection
function toggleRegion(regionId) {
    const card = document.getElementById(`region-${regionId}`);
    const checkbox = card.querySelector('.region-checkbox');
    const slider = card.querySelector('.severity-slider');
    
    regionStates[regionId].selected = !regionStates[regionId].selected;
    checkbox.checked = regionStates[regionId].selected;
    slider.disabled = !regionStates[regionId].selected;
    
    if (regionStates[regionId].selected) {
        card.classList.remove('bg-gray-50', 'border-gray-200');
        card.classList.add('bg-blue-50', 'border-blue-400');
        
        const region = regions.find(r => r.id === regionId);
        markers[regionId].setStyle({
            fillColor: region.color,
            color: region.color,
            fillOpacity: 0.8
        });
    } else {
        card.classList.remove('bg-blue-50', 'border-blue-400');
        card.classList.add('bg-gray-50', 'border-gray-200');
        
        markers[regionId].setStyle({
            fillColor: '#cbd5e1',
            color: '#64748b',
            fillOpacity: 0.6
        });
    }
}

// Update severity value
function updateSeverity(regionId, severity) {
    regionStates[regionId].severity = severity;
    
    const card = document.getElementById(`region-${regionId}`);
    card.querySelector('.severity-value').textContent = severity;
    card.querySelector('.severity-display').textContent = severity;
    
    const color = getSeverityColor(severity);
    card.querySelector('.severity-badge').style.backgroundColor = color;
    card.querySelector('.progress-bar').style.width = `${severity * 10}%`;
    card.querySelector('.progress-bar').style.backgroundColor = color;
}

// Get severity color
function getSeverityColor(severity) {
    if (severity <= 3) return '#10b981'; // Green
    if (severity <= 6) return '#fbbf24'; // Yellow
    if (severity <= 8) return '#f97316'; // Orange
    return '#ef4444'; // Red
}

// Select all regions
function selectAll() {
    regions.forEach(region => {
        if (!regionStates[region.id].selected) {
            toggleRegion(region.id);
        }
    });
}

// Clear all selections
function clearAll() {
    regions.forEach(region => {
        if (regionStates[region.id].selected) {
            toggleRegion(region.id);
        }
    });
}

// Run allocation
async function runAllocation() {
    const selectedRegions = regions
        .filter(r => regionStates[r.id].selected)
        .map(r => r.id);
    
    const severities = selectedRegions.map(id => regionStates[id].severity);

    if (selectedRegions.length === 0) {
        showToast('Please select at least one region', 'error');
        return;
    }

    const payload = {
        regions: selectedRegions,
        severities: severities,
        manualResourceEntry: false,
        manualStock: {}
    };

    console.log('Sending payload:', payload);

    // Show loading state
    document.getElementById('empty-state').classList.add('hidden');
    document.getElementById('results-container').classList.add('hidden');
    document.getElementById('loading-state').classList.remove('hidden');
    document.getElementById('run-allocation-btn').disabled = true;

    try {
        const response = await fetch('/run_with_input', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(payload)
        });

        const data = await response.json();

        if (data.status === 'success') {
            displayResults(data.report);
            showToast('Allocation completed successfully', 'success');
        } else {
            throw new Error(data.message || 'Allocation failed');
        }
    } catch (error) {
        console.error('Error:', error);
        showToast(`Error: ${error.message}`, 'error');
        document.getElementById('loading-state').classList.add('hidden');
        document.getElementById('empty-state').classList.remove('hidden');
    } finally {
        document.getElementById('run-allocation-btn').disabled = false;
    }
}

// Display results
function displayResults(report) {
    document.getElementById('loading-state').classList.add('hidden');
    document.getElementById('results-container').classList.remove('hidden');
    document.getElementById('summary-section').classList.remove('hidden');

    const lines = report.split('\n').filter(line => line.trim() !== '');
    const tbody = document.getElementById('results-tbody');
    tbody.innerHTML = '';

    let totalRequests = 0;
    let totalAllocated = 0;
    let totalCost = 0;

    // Skip header line
    const dataLines = lines.slice(1);

    dataLines.forEach((line, index) => {
        const parts = line.split(',').map(p => p.trim());
        
        if (parts.length >= 8) {
            const [reqId, regionId, resourceId, allocated, route, cost, status, timestamp] = parts;
            
            totalRequests++;
            totalAllocated += parseInt(allocated) || 0;
            totalCost += parseFloat(cost) || 0;

            const row = document.createElement('tr');
            row.className = 'hover:bg-gray-50 animate-slide-in';
            row.style.animationDelay = `${index * 0.05}s`;
            
            const regionName = regions.find(r => r.id === parseInt(regionId))?.name || regionId;
            
            row.innerHTML = `
                <td class="px-4 py-3 text-sm text-gray-900">${reqId}</td>
                <td class="px-4 py-3 text-sm font-medium text-gray-900">${regionName}</td>
                <td class="px-4 py-3 text-sm text-gray-600">${resourceId}</td>
                <td class="px-4 py-3 text-sm text-gray-900 font-semibold">${allocated}</td>
                <td class="px-4 py-3 text-sm text-gray-600">${route}</td>
                <td class="px-4 py-3 text-sm text-gray-900">₹${parseFloat(cost).toFixed(2)}</td>
                <td class="px-4 py-3">
                    <span class="status-badge ${getStatusClass(status)}">${status}</span>
                </td>
                <td class="px-4 py-3 text-sm text-gray-500">${timestamp}</td>
            `;
            
            tbody.appendChild(row);
        }
    });

    // Update summary
    document.getElementById('total-requests').textContent = totalRequests;
    document.getElementById('total-allocated').textContent = totalAllocated;
    document.getElementById('total-cost').textContent = `₹${totalCost.toFixed(2)}`;
}

// Get status badge class
function getStatusClass(status) {
    const statusUpper = status.toUpperCase();
    if (statusUpper.includes('FULL')) {
        return 'bg-green-100 text-green-800';
    } else if (statusUpper.includes('PARTIAL')) {
        return 'bg-yellow-100 text-yellow-800';
    } else if (statusUpper.includes('OUT_OF_STOCK') || statusUpper.includes('FAILED')) {
        return 'bg-red-100 text-red-800';
    } else {
        return 'bg-gray-100 text-gray-800';
    }
}

// Show toast notification
function showToast(message, type = 'info') {
    const toast = document.createElement('div');
    toast.className = `toast ${type === 'success' ? 'bg-green-500' : type === 'error' ? 'bg-red-500' : 'bg-blue-500'} text-white`;
    
    const icon = type === 'success' ? 'check-circle' : type === 'error' ? 'exclamation-circle' : 'info-circle';
    
    toast.innerHTML = `
        <div class="flex items-center space-x-3">
            <i class="fas fa-${icon}"></i>
            <span>${message}</span>
        </div>
    `;
    
    document.body.appendChild(toast);
    
    setTimeout(() => {
        toast.style.opacity = '0';
        setTimeout(() => toast.remove(), 300);
    }, 3000);
}