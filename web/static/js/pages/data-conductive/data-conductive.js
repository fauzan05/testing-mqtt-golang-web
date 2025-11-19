// Data Conductive Page Script
(function(){
    const key = 'core_suit_list';
    function read(){ return JSON.parse(localStorage.getItem(key) || '[]'); }
    function write(arr){ localStorage.setItem(key, JSON.stringify(arr)); }

    function escape(s){ return String(s||'').replace(/[&<>"']/g,function(c){return{'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":"&#39;"}[c]}); }

    // Modal functions
    const formModal = document.getElementById('formModal');
    const modalTitle = document.getElementById('modalTitle');
    
    function openModal(){
        formModal.classList.add('show');
        document.body.style.overflow = 'hidden';
    }
    
    function closeModal(){
        formModal.classList.remove('show');
        document.body.style.overflow = 'auto';
        // reset editing mode when closing
        editingIndex = -1;
        document.getElementById('addBtn').textContent = 'Simpan';
        modalTitle.textContent = 'Input Data Baju Konduktif';
        // clear form
        clearForm();
    }
    
    function clearForm(){
        document.getElementById('nama').value = '';
        document.getElementById('kode').value = '';
        document.getElementById('unit').value = '';
        document.getElementById('ukuran').value = '';
        document.getElementById('tahun').value = '';
    }
    
    // Open modal when clicking "+ Tambah Data"
    document.getElementById('toggleFormBtn').addEventListener('click', function(){
        openModal();
    });
    
    // Close modal buttons
    document.getElementById('closeModalBtn').addEventListener('click', closeModal);
    document.getElementById('cancelBtn').addEventListener('click', closeModal);
    
    // Close modal when clicking outside
    formModal.addEventListener('click', function(e){
        if(e.target === formModal){
            closeModal();
        }
    });

    let editingIndex = -1; // Track which item is being edited

    function render(){
        const tbody = document.querySelector('#table tbody');
        const arr = read();
        tbody.innerHTML = '';
        arr.forEach((it, idx)=>{
            const tr = document.createElement('tr');
            tr.innerHTML = `
                <td style="padding:6px;border-bottom:1px solid #eee">${idx+1}</td>
                <td style="padding:6px;border-bottom:1px solid #eee">${escape(it.nama)}</td>
                <td style="padding:6px;border-bottom:1px solid #eee">${escape(it.kode)}</td>
                <td style="padding:6px;border-bottom:1px solid #eee">${escape(it.unit)}</td>
                <td style="padding:6px;border-bottom:1px solid #eee">${escape(it.ukuran)}</td>
                <td style="padding:6px;border-bottom:1px solid #eee">${escape(it.tahun)}</td>
                <td style="padding:6px;border-bottom:1px solid #eee">
                    <button data-idx="${idx}" class="edit" style="background:#ffa500;color:#fff;padding:4px 8px;border-radius:6px;border:none;margin-right:4px">Edit</button>
                    <button data-idx="${idx}" class="del" style="background:#e74c3c;color:#fff;padding:4px 8px;border-radius:6px;border:none">Hapus</button>
                </td>
            `;
            tbody.appendChild(tr);
        });
    }

    document.getElementById('addBtn').addEventListener('click', function(){
        const nama = document.getElementById('nama').value.trim();
        const kode = document.getElementById('kode').value.trim();
        const unit = document.getElementById('unit').value.trim();
        const ukuran = document.getElementById('ukuran').value.trim();
        const tahun = document.getElementById('tahun').value.trim();
        if(!nama || !kode){ alert('Nama dan Kode wajib diisi'); return; }
        
        const arr = read();
        
        if(editingIndex >= 0){
            // Update existing item
            arr[editingIndex] = { nama, kode, unit, ukuran, tahun };
            write(arr);
            render();
            alert('Data berhasil diupdate!');
            editingIndex = -1;
            document.getElementById('addBtn').textContent = 'Simpan';
            modalTitle.textContent = 'Input Data Baju Konduktif';
        } else {
            // Add new item
            arr.push({ nama, kode, unit, ukuran, tahun });
            write(arr);
            render();
            alert('Data berhasil disimpan!');
        }
        
        // clear form and close modal
        clearForm();
        closeModal();
    });

    document.getElementById('clearBtn').addEventListener('click', function(){
        clearForm();
    });

    document.body.addEventListener('click', function(e){
        // Handle Edit button
        if(e.target && e.target.matches('.edit')){
            const idx = Number(e.target.getAttribute('data-idx'));
            const arr = read();
            const item = arr[idx];
            
            // Fill form with existing data
            document.getElementById('nama').value = item.nama || '';
            document.getElementById('kode').value = item.kode || '';
            document.getElementById('unit').value = item.unit || '';
            document.getElementById('ukuran').value = item.ukuran || '';
            document.getElementById('tahun').value = item.tahun || '';
            
            // Set editing mode
            editingIndex = idx;
            document.getElementById('addBtn').textContent = 'Update';
            modalTitle.textContent = 'Edit Data Baju Konduktif';
            
            // Show modal
            openModal();
        }
        
        // Handle Delete button
        if(e.target && e.target.matches('.del')){
            const idx = Number(e.target.getAttribute('data-idx'));
            if(!confirm('Hapus entri ini?')) return;
            const arr = read(); 
            arr.splice(idx,1); 
            write(arr); 
            render();
            alert('Data berhasil dihapus!');
        }
    });

    render();
})();
