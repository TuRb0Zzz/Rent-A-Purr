import React, { useState } from 'react';
import { createPortal } from 'react-dom';
import { X, PlusCircle } from 'lucide-react';
import { getImageUrl } from '../api'; // Чтобы показать аватарку кота

const COLOR_PRESETS =[
    { id: 'purple', bg: '#7838F5', color: '#FFFFFF' },
    { id: 'cyan', bg: '#00C4D1', color: '#FFFFFF' },
    { id: 'green', bg: '#00D26A', color: '#FFFFFF' },
    { id: 'coral', bg: '#F6828C', color: '#FFFFFF' },
    { id: 'red', bg: '#FF2B4A', color: '#FFFFFF' },
    { id: 'yellow', bg: '#FFB300', color: '#FFFFFF' }
];

// Дефолтные опции (можно дополнить теми, что уже есть у кота)
const DEFAULT_MEDICAL_OPTIONS =[
    { iconName: 'syringe', label: 'Привит', bg: '#FF2B4A', color: '#FFFFFF' },
    { iconName: 'heartPulse', label: 'Здоров', bg: '#00D26A', color: '#FFFFFF' },
    { iconName: 'microchip', label: 'Чипирован', bg: '#00C4D1', color: '#FFFFFF' }
];

export default function EditCat({ cat, onClose, onSubmit }) {
    const [tags, setTags] = useState((cat.tags ||[]).join(', '));

    // Инициализируем выбранную медицину тем, что уже есть у кота
    const[selectedMedical, setSelectedMedical] = useState(cat.medical ||[]);

    // Собираем все опции: дефолтные + те, что уже есть у кота (чтобы они отображались как кнопки)
    const initialCustomOptions = (cat.medical ||[]).filter(cm => !DEFAULT_MEDICAL_OPTIONS.find(dm => dm.label === cm.label));
    const [customMedical, setCustomMedical] = useState(initialCustomOptions);

    const [newMedLabel, setNewMedLabel] = useState('');
    const[newMedColor, setNewMedColor] = useState(COLOR_PRESETS[0]);

    // Обработка медицины
    const toggleMedical = (medObj) => {
        setSelectedMedical(prev => {
            const exists = prev.find(m => m.label === medObj.label);
            if (exists) return prev.filter(m => m.label !== medObj.label);
            return [...prev, medObj];
        });
    };

    const handleAddCustomMedical = () => {
        if (!newMedLabel.trim()) return;
        const newMed = {
            iconName: 'stethoscope',
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
            ...cat,
            tags: tags.split(',').map(tag => tag.trim()).filter(Boolean),
            medical: selectedMedical
        });
    };

    const allMedicalOptions =[...DEFAULT_MEDICAL_OPTIONS, ...customMedical];

    return createPortal(
        <div className="fixed inset-0 z-[9999] flex items-center justify-center p-4 bg-slate-900/40 backdrop-blur-sm animate-in fade-in duration-200 font-m3">
            <div className="bg-white rounded-[32px] w-full max-w-2xl shadow-2xl border-2 border-[#7838F5]/20 overflow-hidden flex flex-col max-h-[90vh]">

                <div className="px-8 py-6 border-b-2 border-slate-100 flex justify-between items-center bg-white shrink-0">
                    <h2 className="text-3xl font-extrabold text-slate-800 tracking-tight">Редактирование профиля</h2>
                    <button onClick={onClose} className="p-2 text-slate-400 hover:text-slate-800 hover:bg-slate-100 rounded-full transition-colors"><X size={28} strokeWidth={2.5} /></button>
                </div>

                <div className="p-8 overflow-y-auto no-scrollbar flex-1">

                    {/* Инфо профиля (Read-only для красивого отображения) */}
                    <div className="flex items-center gap-6 mb-8 bg-slate-50 p-4 rounded-[24px] border border-slate-100">
                        <img src={getImageUrl(cat.filenames)} alt={cat.name} className="w-24 h-24 rounded-2xl object-cover shadow-sm" />
                        <div>
                            <h3 className="text-2xl font-extrabold text-slate-800">{cat.name}</h3>
                            <p className="text-slate-500 font-bold">{cat.breed} • {cat.age}</p>
                            <p className="text-xs text-slate-400 mt-1 uppercase tracking-widest font-bold">Основные данные менять нельзя</p>
                        </div>
                    </div>

                    <form id="edit-cat-form" onSubmit={handleSubmit} className="space-y-8">

                        <div className="space-y-2">
                            <label className="text-sm font-extrabold text-slate-700 ml-1 uppercase">Характеристики (через запятую)</label>
                            <input value={tags} onChange={e => setTags(e.target.value)} className="w-full px-5 py-4 rounded-2xl bg-white border-2 border-slate-200 focus:border-[#7838F5] outline-none font-bold" placeholder="Ласковый, Игривый" />
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

                            <div className="flex flex-col gap-3 mt-4 p-5 bg-slate-50 border-2 border-slate-100 rounded-[24px]">
                                <label className="text-xs font-extrabold text-slate-500 uppercase tracking-wider">Создать новый статус</label>
                                <div className="flex flex-col md:flex-row gap-4 items-start md:items-center">
                                    <input type="text" value={newMedLabel} onChange={(e) => setNewMedLabel(e.target.value)} placeholder="Напр. Кастрирован" className="flex-grow w-full px-5 py-3 rounded-xl border-2 border-slate-200 bg-white focus:border-[#7838F5] outline-none transition-colors font-bold text-slate-800" />
                                    <div className="flex items-center gap-3 px-1">
                                        {COLOR_PRESETS.map(preset => (
                                            <button type="button" key={preset.id} onClick={() => setNewMedColor(preset)} style={{ backgroundColor: preset.bg }} className={`w-8 h-8 rounded-full transition-transform border-2 ${newMedColor.id === preset.id ? 'border-slate-800 scale-110 shadow-md' : 'border-transparent opacity-50 hover:opacity-100 hover:scale-110'}`} />
                                        ))}
                                    </div>
                                    <button type="button" onClick={handleAddCustomMedical} disabled={!newMedLabel.trim()} className="px-5 py-3.5 bg-slate-200 text-slate-600 rounded-xl font-extrabold hover:bg-slate-300 disabled:opacity-50 transition-colors flex gap-2 items-center">
                                        <PlusCircle size={20} />Добавить
                                    </button>
                                </div>
                            </div>
                        </div>

                    </form>
                </div>

                <div className="px-8 py-5 border-t-2 border-slate-100 bg-white flex justify-end gap-3 shrink-0 rounded-b-[32px]">
                    <button type="button" onClick={onClose} className="px-6 py-3.5 rounded-2xl text-slate-500 font-extrabold hover:bg-slate-100 transition-colors">Отмена</button>
                    <button form="edit-cat-form" type="submit" className="px-8 py-3.5 rounded-2xl bg-[#7838F5] hover:bg-[#6025D1] text-white font-extrabold shadow-lg shadow-[#7838F5]/30 text-lg">Сохранить</button>
                </div>
            </div>
        </div>,
        document.body
    );
}