import React, { useState } from 'react';
import { createPortal } from 'react-dom';
import { X, Upload, PlusCircle, Check } from 'lucide-react';

const COLOR_PRESETS =[
    { id: 'purple', bg: '#7838F5', color: '#FFFFFF' },
    { id: 'cyan', bg: '#00C4D1', color: '#FFFFFF' },
    { id: 'green', bg: '#00D26A', color: '#FFFFFF' },
    { id: 'coral', bg: '#F6828C', color: '#FFFFFF' },
    { id: 'red', bg: '#FF2B4A', color: '#FFFFFF' },
    { id: 'yellow', bg: '#FFB300', color: '#FFFFFF' }
];

const MEDICAL_OPTIONS =[
    { iconName: 'syringe', label: 'Привит', bg: '#FF2B4A', color: '#FFFFFF' },
    { iconName: 'heartPulse', label: 'Здоров', bg: '#00D26A', color: '#FFFFFF' },
    { iconName: 'microchip', label: 'Чипирован', bg: '#00C4D1', color: '#FFFFFF' }
];

export default function AddCat({ onClose, onSubmit }) {
    const [formData, setFormData] = useState({ name: '', breed: '', age: '', description: '', tags: '' });

    // Состояния для фото и медицины
    const [photos, setPhotos] = useState([]);
    const [selectedMedical, setSelectedMedical] = useState([]);
    const[customMedical, setCustomMedical] = useState([]);

    const [newMedLabel, setNewMedLabel] = useState('');
    const[newMedColor, setNewMedColor] = useState(COLOR_PRESETS[0]);

    // Обработка файлов
    const handleFileChange = (e) => {
        if (e.target.files) {
            setPhotos(prev => [...prev, ...Array.from(e.target.files)]);
        }
    };

    const removePhoto = (index) => {
        setPhotos(prev => prev.filter((_, i) => i !== index));
    };

    // Обработка медицины
    const toggleMedical = (medObj) => {
        setSelectedMedical(prev => {
            const exists = prev.find(m => m.label === medObj.label);
            if (exists) return prev.filter(m => m.label !== medObj.label);
            return[...prev, medObj];
        });
    };

    const handleAddCustomMedical = () => {
        if (!newMedLabel.trim()) return;
        const newMed = {
            iconName: 'stethoscope', // дефолтная иконка для кастомных
            label: newMedLabel.trim(),
            color: newMedColor.color,
            bg: newMedColor.bg
        };
        setCustomMedical([...customMedical, newMed]);
        setSelectedMedical([...selectedMedical, newMed]);
        setNewMedLabel('');
    };

    const handleSubmit = (e) => {
        e.preventDefault();
        onSubmit({
            ...formData,
            tags: formData.tags.split(',').map(tag => tag.trim()).filter(Boolean),
            photos: photos,
            medical: selectedMedical
        });
    };

    const allMedicalOptions =[...MEDICAL_OPTIONS, ...customMedical];

    return createPortal(
        <div className="fixed inset-0 z-[9999] flex items-center justify-center p-4 bg-slate-900/40 backdrop-blur-sm animate-in fade-in duration-200 font-m3">
            <div className="bg-white rounded-[32px] w-full max-w-3xl shadow-2xl border-2 border-[#F6828C]/20 overflow-hidden flex flex-col max-h-[95vh]">

                <div className="px-8 py-6 border-b-2 border-slate-100 flex justify-between items-center bg-white shrink-0">
                    <h2 className="text-3xl font-extrabold text-slate-800 tracking-tight">Добавить нового кота</h2>
                    <button onClick={onClose} className="p-2 text-slate-400 hover:text-slate-800 hover:bg-slate-100 rounded-full transition-colors"><X size={28} strokeWidth={2.5} /></button>
                </div>

                <div className="p-8 overflow-y-auto no-scrollbar flex-1">
                    <form id="add-cat-form" onSubmit={handleSubmit} className="space-y-8">

                        {/* Основная инфа */}
                        <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
                            <div className="space-y-2"><label className="text-sm font-extrabold text-slate-700 ml-1 uppercase">Имя кота *</label><input required value={formData.name} onChange={e => setFormData({...formData, name: e.target.value})} className="w-full px-5 py-4 rounded-2xl bg-slate-50 border-2 border-slate-100 focus:border-[#F6828C] outline-none font-bold" placeholder="Барсик"/></div>
                            <div className="space-y-2"><label className="text-sm font-extrabold text-slate-700 ml-1 uppercase">Порода *</label><input required value={formData.breed} onChange={e => setFormData({...formData, breed: e.target.value})} className="w-full px-5 py-4 rounded-2xl bg-slate-50 border-2 border-slate-100 focus:border-[#F6828C] outline-none font-bold" placeholder="Мейн-кун"/></div>
                            <div className="space-y-2"><label className="text-sm font-extrabold text-slate-700 ml-1 uppercase">Возраст *</label><input required value={formData.age} onChange={e => setFormData({...formData, age: e.target.value})} className="w-full px-5 py-4 rounded-2xl bg-slate-50 border-2 border-slate-100 focus:border-[#F6828C] outline-none font-bold" placeholder="2 года"/></div>
                            <div className="space-y-2"><label className="text-sm font-extrabold text-slate-700 ml-1 uppercase">Характеристики (через запятую)</label><input value={formData.tags} onChange={e => setFormData({...formData, tags: e.target.value})} className="w-full px-5 py-4 rounded-2xl bg-slate-50 border-2 border-slate-100 focus:border-[#F6828C] outline-none font-bold" placeholder="Ласковый, Игривый" /></div>
                        </div>

                        <div className="space-y-2">
                            <label className="text-sm font-extrabold text-slate-700 ml-1 uppercase">Описание *</label>
                            <textarea required value={formData.description} onChange={e => setFormData({...formData, description: e.target.value})} rows="3" className="w-full px-5 py-4 rounded-2xl bg-slate-50 border-2 border-slate-100 focus:border-[#F6828C] outline-none font-bold resize-none" placeholder="Расскажите о котике..."></textarea>
                        </div>

                        {/* Блок фотографий */}
                        <div className="space-y-3">
                            <label className="text-sm font-extrabold text-slate-700 ml-1 uppercase">Фотографии ({photos.length})</label>

                            <div className="flex flex-wrap gap-4 items-start">
                                {photos.map((file, index) => (
                                    <div key={index} className="relative w-28 h-28 rounded-2xl overflow-hidden shadow-sm group border-2 border-slate-100">
                                        <img src={URL.createObjectURL(file)} alt="preview" className="w-full h-full object-cover" />
                                        <button type="button" onClick={() => removePhoto(index)} className="absolute top-1 right-1 p-1 bg-black/50 text-white rounded-full opacity-0 group-hover:opacity-100 transition-opacity"><X size={14}/></button>
                                    </div>
                                ))}

                                {/* Кнопка добавления файла */}
                                <div className="relative w-28 h-28 rounded-2xl bg-slate-50 border-2 border-dashed border-slate-300 hover:border-[#F6828C] hover:bg-[#F6828C]/5 transition-colors flex flex-col items-center justify-center cursor-pointer group">
                                    <input type="file" multiple accept="image/*" onChange={handleFileChange} className="absolute inset-0 w-full h-full opacity-0 cursor-pointer" />
                                    <Upload size={24} className="text-slate-400 group-hover:text-[#F6828C] mb-1" />
                                    <span className="text-xs font-bold text-slate-400 group-hover:text-[#F6828C]">Загрузить</span>
                                </div>
                            </div>
                        </div>

                        {/* Блок Медицины */}
                        <div className="space-y-3 border-t-2 border-slate-100 pt-6">
                            <label className="text-sm font-extrabold text-slate-700 ml-1 uppercase">Медицинская карта</label>

                            <div className="flex flex-wrap gap-3">
                                {allMedicalOptions.map(opt => {
                                    const isSelected = selectedMedical.some(m => m.label === opt.label);
                                    return (
                                        <button type="button" key={opt.label} onClick={() => toggleMedical(opt)} style={isSelected ? { backgroundColor: opt.bg, color: opt.color, borderColor: opt.bg } : {}} className={`px-4 py-2.5 rounded-2xl text-sm font-extrabold border-2 transition-colors shadow-sm ${!isSelected && 'border-slate-200 bg-white text-slate-500 hover:bg-slate-50'}`}>
                                            {opt.label}
                                        </button>
                                    );
                                })}
                            </div>

                            {/* Создание кастомного мед. статуса */}
                            <div className="flex flex-col gap-3 mt-4 p-5 bg-slate-50 border-2 border-slate-100 rounded-[24px]">
                                <label className="text-xs font-extrabold text-slate-500 uppercase tracking-wider">Создать новый статус</label>
                                <div className="flex flex-col md:flex-row gap-4 items-start md:items-center">
                                    <input type="text" value={newMedLabel} onChange={(e) => setNewMedLabel(e.target.value)} placeholder="Напр. Обработка от клещей" className="flex-grow w-full px-5 py-3 rounded-xl border-2 border-slate-200 bg-white focus:border-[#F6828C] outline-none transition-colors font-bold text-slate-800" />
                                    <div className="flex items-center gap-3 px-1">
                                        {COLOR_PRESETS.map(preset => (
                                            <button type="button" key={preset.id} onClick={() => setNewMedColor(preset)} style={{ backgroundColor: preset.bg }} className={`w-8 h-8 rounded-full transition-transform border-2 ${newMedColor.id === preset.id ? 'border-slate-800 scale-110 shadow-md' : 'border-transparent opacity-50 hover:opacity-100 hover:scale-110'}`} />
                                        ))}
                                    </div>
                                    <button type="button" onClick={handleAddCustomMedical} disabled={!newMedLabel.trim()} className="px-5 py-3.5 bg-slate-200 text-slate-600 rounded-xl font-extrabold hover:bg-slate-300 disabled:opacity-50 disabled:cursor-not-allowed flex items-center gap-2 transition-colors">
                                        <PlusCircle size={20} />Добавить
                                    </button>
                                </div>
                            </div>
                        </div>

                    </form>
                </div>

                <div className="px-8 py-5 border-t-2 border-slate-100 bg-white flex justify-end gap-3 shrink-0 rounded-b-[32px]">
                    <button type="button" onClick={onClose} className="px-6 py-3.5 rounded-2xl text-slate-500 font-extrabold hover:bg-slate-100 transition-colors">Отмена</button>
                    <button form="add-cat-form" type="submit" className="px-8 py-3.5 rounded-2xl bg-[#F6828C] hover:bg-[#FF2B4A] text-white font-extrabold shadow-lg shadow-[#F6828C]/30 text-lg">Добавить кота</button>
                </div>
            </div>
        </div>,
        document.body
    );
}