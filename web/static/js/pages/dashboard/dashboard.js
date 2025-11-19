// Dashboard Chart Script
(function() {
    // Labels for X axis
    const labels = ['R1','R2','R3','R4','R5','R6','R7','R8'];

    // Utility to generate visually distinct colors
    function randomColor(alpha=1){
        const palette = [
            'rgba(10, 102, 194,'+alpha+')', // blue
            'rgba(220,53,69,'+alpha+')', // red
            'rgba(40,167,69,'+alpha+')', // green
            'rgba(255,193,7,'+alpha+')', // yellow
            'rgba(108,117,125,'+alpha+')', // gray
            'rgba(123,31,162,'+alpha+')', // purple
            'rgba(0,150,136,'+alpha+')' // teal
        ];
        return palette[Math.floor(Math.random()*palette.length)];
    }

    // Data per person: each person has 5 pengujian (R1..R8)
    const personData = {
        'Muhammad Wahyudi': [
            { label: 'Pengujian 1', data:[145, 320, 260, 410, 360, 420, 335, 480] },
            { label: 'Pengujian 2', data:[150, 290, 310, 370, 345, 395, 360, 330] },
            { label: 'Pengujian 3', data:[140, 200, 255, 230, 415, 380, 410, 390] },
            { label: 'Pengujian 4', data:[155, 340, 290, 460, 420, 355, 500, 445] },
            { label: 'Pengujian 5', data:[148, 310, 275, 330, 495, 360, 425, 370] }
        ],
        'Andi Arjuna': [
            { label: 'Pengujian 1', data:[320, 280, 410, 270, 510, 187, 240, 210] },
            { label: 'Pengujian 2', data:[300, 360, 290, 250, 430, 180, 480, 200] },
            { label: 'Pengujian 3', data:[340, 305, 320, 455, 280, 200, 260, 230] },
            { label: 'Pengujian 4', data:[310, 270, 560, 275, 265, 190, 245, 215] },
            { label: 'Pengujian 5', data:[330, 290, 305, 285, 615, 205, 255, 225] }
        ],
        'Aji Nugraha Yusuf': [
            { label: 'Pengujian 1', data:[50, 180, 120, 200, 310, 420, 520, 640] },
            { label: 'Pengujian 2', data:[60, 90, 430, 210, 320, 430, 540, 350] },
            { label: 'Pengujian 3', data:[55, 85, 125, 605, 315, 425, 530, 645] },
            { label: 'Pengujian 4', data:[70, 100, 140, 220, 330, 440, 360, 670] },
            { label: 'Pengujian 5', data:[65, 95, 135, 215, 325, 880, 550, 660] }
        ],
        'Okky Andreas T': [
            { label: 'Pengujian 1', data:[480, 783, 430, 120, 380, 567, 300, 121] },
            { label: 'Pengujian 2', data:[460, 750, 810, 140, 360, 540, 280, 130] },
            { label: 'Pengujian 3', data:[500, 800, 450, 160, 400, 580, 320, 150] },
            { label: 'Pengujian 4', data:[470, 770, 420, 730, 370, 555, 295, 125] },
            { label: 'Pengujian 5', data:[490, 790, 440, 150, 390, 570, 935, 140] }
        ],
        'Dimas Harry LF': [
            { label: 'Pengujian 1', data:[100, 179, 756, 169, 434, 187, 280, 760] },
            { label: 'Pengujian 2', data:[120, 820, 700, 180, 420, 195, 300, 740] },
            { label: 'Pengujian 3', data:[90, 160, 730, 170, 410, 185, 920, 720] },
            { label: 'Pengujian 4', data:[110, 190, 440, 175, 425, 192, 305, 750] },
            { label: 'Pengujian 5', data:[130, 210, 770, 190, 440, 200, 320, 480] }
        ],
        'Fridom Tusano Hadi': [
            { label: 'Pengujian 1', data:[200, 180, 720, 240, 260, 280, 300, 320] },
            { label: 'Pengujian 2', data:[210, 900, 230, 250, 270, 290, 310, 330] },
            { label: 'Pengujian 3', data:[190, 170, 210, 230, 456, 270, 654, 290] },
            { label: 'Pengujian 4', data:[220, 200, 240, 260, 880, 300, 320, 340] },
            { label: 'Pengujian 5', data:[230, 215, 250, 465, 290, 564, 300, 765] }
        ]
    };

    // color palette for the 5 pengujian
    const pengujianColors = [
        'rgba(10,102,194,0.95)',
        'rgba(220,53,69,0.95)',
        'rgba(40,167,69,0.95)',
        'rgba(255,193,7,0.95)',
        'rgba(108,117,125,0.95)'
    ];

    // helper to map plain personData to Chart.js dataset objects with colors
    function datasetsForPerson(name){
        const raw = personData[name] || [];
        return raw.map((d,i)=>({
            label: d.label,
            data: d.data.slice(),
            borderColor: pengujianColors[i % pengujianColors.length],
            backgroundColor: 'transparent',
            tension: 0.25,
            pointRadius: 4,
            pointHoverRadius:6,
            spanGaps: true
        }));
    }

    // Plugin: draw horizontal threshold line at 200Ω
    const thresholdPlugin = {
        id: 'thresholdLine',
        afterDraw: (chart)=>{
            const yValue = 200;
            const yScale = chart.scales.y;
            if(!yScale) return;
            const y = yScale.getPixelForValue(yValue);
            const ctx = chart.ctx;
            ctx.save();
            ctx.beginPath();
            ctx.moveTo(chart.chartArea.left, y);
            ctx.lineTo(chart.chartArea.right, y);
            ctx.lineWidth = 1.5;
            ctx.strokeStyle = 'rgba(200, 30, 30, 0.8)';
            ctx.setLineDash([6,4]);
            ctx.stroke();
            ctx.fillStyle = 'rgba(200,30,30,0.9)';
            ctx.font = '12px sans-serif';
            ctx.fillText('Batas bagus 200Ω', chart.chartArea.left + 6, y - 6);
            ctx.restore();
        }
    };

    const ctx = document.getElementById('resistanceChart').getContext('2d');
    // initialize chart with the first person selected
    const defaultPerson = document.getElementById('personSelect').value;
    const resistanceChart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: labels,
            datasets: datasetsForPerson(defaultPerson)
        },
        options: {
            responsive: true,
            maintainAspectRatio: true,
            plugins: {
                legend: { position: 'top' },
                tooltip: {
                    mode: 'nearest',
                    intersect: false,
                    callbacks: {
                        label: function(context){
                            return `${context.dataset.label}: ${context.formattedValue} Ω`;
                        }
                    }
                }
            },
            scales: {
                x: {
                    title: { display: true, text: 'Tahanan Baju Konduktif (R1 - R8)' },
                    grid: { display: false }
                },
                y: {
                    min: 0,
                    max: 1000,
                    ticks: { stepSize: 100 },
                    title: { display: true, text: 'Nilai Tahanan (Ω)' }
                }
            }
        },
        plugins: [thresholdPlugin]
    });
    
    // Update chart when a different person is selected
    document.getElementById('personSelect').addEventListener('change', (e)=>{
        const name = e.target.value;
        resistanceChart.data.datasets = datasetsForPerson(name);
        resistanceChart.update();
    });
})();
